#include "Engine.h"

const int DATABASE_MAX_SIZE = 6400;
const int CONST_INF = 10000;

const int BATCH_SIZE = 144;

Engine::Engine()
	: BatchSize(BATCH_SIZE), InputTensor(make_shape(BATCH_SIZE, 8, 8, 14)),
	OutputMoveTensor(make_shape(BATCH_SIZE, 2, 64)), OutputEvalTensor(make_shape(BATCH_SIZE, 1)),
	MoveTensor(make_shape(BATCH_SIZE, 64 + 64 + 6 + 7 + 7)), SortTensor(make_shape(BATCH_SIZE, 1))
{
	Database = new Data[DATABASE_MAX_SIZE];
	DBCounter = 0;
	DBSize = 0;

	NetPlay = new EvalNet(1);
	NetTrain = new EvalNet(BatchSize);
	NetSort = new SortNet(BatchSize);

	for (int i = 0; i<2; i++)
	{
		for (int j = 0; j<100; j++)
		{
			KillerMoves[i][j] = createNullMove(CurrentPos.EPSquare);
		}
	}
	for (int i = 0; i<64; i++)
	{
		for (int j = 0; j<64; j++)
		{
			HistoryScores[i][j] = 0;
		}
	}

	Table = new TranspositionTable(4096);

	NodeCount = 0;
	SortNetCount = 0;
	BetaCutoffCount = 0;
	BetaCutoffValue = 0;
}

Engine::~Engine()
{
	delete[] Database;
	delete Table;

	if(NetTrain)
		delete NetTrain;
	if(NetPlay)
		delete NetPlay;
	if (NetSort)
		delete NetSort;

	Table = NULL;
	Database = NULL;
	NetTrain = NULL;
	NetPlay = NULL;
	NetSort = NULL;
}

void Engine::load_nets(std::string path)
{
	if(NetTrain)
		NetTrain->load(path);

	if(NetPlay)
		NetPlay->load(path);
}

void Engine::learn_eval(uint64_t num_games)
{
	uint64_t c = 0;
	for (uint64_t i = 0; i < num_games; i++)
	{
		printf("Game: %d\n", i + 1);
		CurrentPos.setStartPos();
		while (true)
		{
			std::vector<Move> moves;
			moves.reserve(128);
			CurrentPos.generateMoves(moves);

			Move m = moves[rand() % moves.size()];
			while (!CurrentPos.isMoveLegal(m))
			{
				m = moves[rand() % moves.size()];
			}

			int leaf = LeafEval_MatOnly();

			int r = rand() % 10;
			if (r == 0)
			{
				Data* d = &Database[DBCounter];
				d->pos.copyFromPosition(CurrentPos);
				if (CurrentPos.Turn == COLOR_BLACK)
				{
					leaf = -leaf;
				}
				d->eval = leaf;
				moveToTensor(m, &d->move);

				DBCounter++;
				if (DBCounter == DATABASE_MAX_SIZE)
				{
					DBCounter = 0;
				}

				if (DBSize < DATABASE_MAX_SIZE)
				{
					DBSize++;
				}
			}

			if (DBSize == DATABASE_MAX_SIZE && c%64==0)
			{
				//printf("EPOCH\n");
				for (int epoch = 0; epoch < 100; epoch++)
				{
					Float error = 0;
					for (int batch = 0; batch < DBSize / BatchSize; batch++)
					{
						for (uint64_t i = 0; i < BatchSize; i++)
						{
							//size_t id = rand() % DBSize;
							size_t id = batch*BatchSize + i;
							memcpy(&InputTensor(i * 8 * 8 * 14), Database[id].pos.Squares.mData, sizeof(Float) * 8 * 8 * 14);
							//memcpy(&OutputTensor(i * 2 * 64), Database[id].move.mData, sizeof(Float) * 2 * 64);
							OutputEvalTensor(i) = Database[id].eval / 100.0;
						}
						for (int run = 0; run < 1; run++)
						{
							error += NetTrain->train(InputTensor, &OutputEvalTensor, nullptr);
							//printf("Error: %f\n", error);
						}
					}
					if (epoch == 99)
					{
						printf("Final error: %f, avg: %f\n", error, error/(DBSize*10));
					}
				}
				
			}

			CurrentPos.makeMove(m);
			if (CurrentPos.getGameStatus() != STATUS_NOTOVER || CurrentPos.movelist.size() > 100)
			{
				break;
			}
			c++;
		}
	}
}

void Engine::learn_eval_NN(uint64_t num_games, double time_limit)
{
	uint64_t c = 0;
	Clock timer;
	timer.Start();
	for (uint64_t i = 0; i < num_games; i++)
	{
		printf("Game: %d\n", i + 1);
		CurrentPos.setStartPos();
		while (true)
		{
			timer.Stop();
			if (timer.ElapsedSeconds() >= time_limit)
				return;

			Move m = createNullMove(CurrentPos.EPSquare);
			int eval = 0;

			int r1 = rand() % 100;
			if (r1 < 75)
			{
				std::vector<Move> moves;
				moves.reserve(128);
				CurrentPos.generateMoves(moves);

				m = moves[rand() % moves.size()];
				while (!CurrentPos.isMoveLegal(m))
				{
					m = moves[rand() % moves.size()];
				}
				assert(m.isNullMove() == false);

				eval = LeafEval();
				if (CurrentPos.Turn == COLOR_BLACK)
				{
					eval = -eval;
				}
			}
			else
			{
				Bitset hash = CurrentPos.HashKey;

				SearchResult go = go_alphabeta(4 + (rand()%2));
				m = go.m;
				eval = go.eval;
				if (CurrentPos.Turn == COLOR_BLACK)
				{
					eval = -eval;
				}
				assert(m.isNullMove() == false);
				assert(CurrentPos.HashKey == hash);
			}
			//assert(m.isNullMove() == false);

			int r2 = rand() % 3;
			if (r2 == 0)
			{
				Data* d = &Database[DBCounter];
				d->pos.copyFromPosition(CurrentPos);
				d->eval = eval;
				moveToTensor(m, &d->move);

				DBCounter++;
				if (DBCounter == DATABASE_MAX_SIZE)
				{
					DBCounter = 0;
					printf("Counter reset\n");
				}

				if (DBSize < DATABASE_MAX_SIZE)
				{
					DBSize++;
				}
			}

			if (DBSize >= DATABASE_MAX_SIZE && c % 64 == 0)
			{
				//printf("EPOCH\n");
				for (int epoch = 0; epoch < 10; epoch++)
				{
					Float error = 0;
					for (int batch = 0; batch < DBSize/BatchSize; batch++)
					{
						for (uint64_t i = 0; i < BatchSize; i++)
						{
							//size_t id = rand() % DBSize;
							size_t id = batch*BatchSize + i;
							memcpy(&InputTensor(i * 8 * 8 * 14), Database[id].pos.Squares.mData, sizeof(Float) * 8 * 8 * 14);
							//memcpy(&OutputTensor(i * 2 * 64), Database[id].move.mData, sizeof(Float) * 2 * 64);
							OutputEvalTensor(i) = Database[id].eval / 100.0;
						}
						for (int run = 0; run < 1; run++)
						{
							error += NetTrain->train(InputTensor, &OutputEvalTensor, nullptr);
							//printf("Error: %f\n", mNet->train(InputTensor, nullptr, &OutputEvalTensor));
						}
					}
					if (epoch == 9)
					{
						printf("Final error: %f, avg: %f\n", error, error / (10 * DBSize));
					}
				}
			}

			CurrentPos.makeMove(m);
			if (CurrentPos.getGameStatus() != STATUS_NOTOVER || CurrentPos.movelist.size() > 100)
			{
				break;
			}
			c++;
		}
	}
}

void Engine::learn_eval_TD(uint64_t num_games, double time_limit)
{
	TimeMode = MODE_DEPTH;

	uint64_t c = 0;
	Clock timer;
	timer.Start();

	int epsilon = 75;
	int my_side = 0;

	float wins = 0;

	int start_pos_id;

	for (uint64_t i = 0; i < num_games; i++)
	{
		printf("Game: %d\n", i + 1);
		if (i != 0)
			printf("wr: %f\n", (wins*1.0) / i);
		CurrentPos.setStartPos();
		//DBSize = 0;
		//DBCounter = 0;
		my_side = (my_side + 1) % 2;
		start_pos_id = DBCounter;
		while (true)
		{
			timer.Stop();
			if (timer.ElapsedSeconds() >= time_limit)
				return;

			Move m = createNullMove(CurrentPos.EPSquare);
			Float eval = 0.0;

			//Make a move
			int r1 = rand() % 100;
			if (r1 < epsilon && CurrentPos.Turn != my_side)
			{
				//printf("rand\n");
				std::vector<Move> moves;
				moves.reserve(128);
				CurrentPos.generateMoves(moves);

				m = moves[rand() % moves.size()];
				while (!CurrentPos.isMoveLegal(m))
				{
					m = moves[rand() % moves.size()];
				}
				assert(m.isNullMove() == false);

				//GoReturn go = go_alphabeta(2);

				eval = LeafEval_NN() / 100.0;
				if (CurrentPos.Turn == COLOR_BLACK)
				{
					eval = -eval;
				}
			}
			else
			{
				//printf("n\n");
				Bitset hash = CurrentPos.HashKey;

				SearchResult go = go_alphabeta(2 + (rand()%2));
				
				m = go.m;
				eval = LeafEval_MatOnly() / 100.0;
				if (CurrentPos.Turn == COLOR_BLACK)
				{
					eval = -eval;
				}
				assert(m.isNullMove() == false);
				assert(CurrentPos.HashKey == hash);
			}
			assert(eval >= -CONST_INF / 100 && eval <= CONST_INF / 100);

			//Save position
			if (my_side == CurrentPos.Turn)
			{
				Data* d = &Database[DBCounter];
				d->pos.copyFromPosition(CurrentPos);
				d->eval = eval;
				moveToTensor(m, &d->move);
				//assert(abs(eval) != 100);

				DBCounter++;
				if (DBCounter == DATABASE_MAX_SIZE)
				{
					DBCounter = 0;
					start_pos_id = 0;
					printf("Counter reset: %d\n", CurrentPos.movelist.size());
				}

				if (DBSize < DATABASE_MAX_SIZE)
				{
					DBSize++;
				}
			}
			
			assert(m.isNullMove() == false);
			CurrentPos.makeMove(m);
			int status = CurrentPos.getGameStatus();
			if (status != STATUS_NOTOVER || CurrentPos.movelist.size()>100)
			{
				if (status == STATUS_WHITEMATED && my_side == COLOR_BLACK)
					wins += 1;
				if (status == STATUS_BLACKMATED && my_side == COLOR_WHITE)
					wins += 1;
				if (isStatusDraw(status))
					wins += 0.5;
				if (CurrentPos.movelist.size() > 100)
				{
					wins += 0.5;
				}
				break;
			}
			c++;
		}

		Float gamma = 0.1;
		for (uint64_t i = start_pos_id; i < DBSize - 1; i++)
		{
			Float current_gamma = 0.0;
			Float sum = 0.0;
			for (uint64_t j = i; j < DBSize - 1; j++)
			{
				//printf("%d %d %f %f %f\n", i, j, Database[j + 1].eval, Database[j].eval, sum);
				sum += current_gamma*(Database[j + 1].eval - Database[j].eval);
				current_gamma *= gamma;
			}
			printf("eval: %f, sum: %f\n", Database[i].eval, sum + Database[i].eval);
			Database[i].eval = Database[i].eval + sum;
		}
	
		//Train
		int num_epochs = 10;
		int num_runs = 1;
		Float error = 0;
		for (int epoch = 0; epoch < num_epochs; epoch++)
		{
			error = 0;
			for (int batch = 0; batch < DBSize / BatchSize; batch++)
			{
				for (uint64_t i = 0; i < BatchSize; i++)
				{
					size_t id = batch*BatchSize + i;
					memcpy(&InputTensor(i * 8 * 8 * 14), Database[id].pos.Squares.mData, sizeof(Float) * 8 * 8 * 14);
					//memcpy(&OutputMoveTensor(i * 2 * 64), Database[id].move.mData, sizeof(Float) * 2 * 64);

					OutputEvalTensor(i) = Database[id].eval;
				}
				for (int run = 0; run < num_runs; run++)
				{
					error += NetTrain->train(InputTensor, &OutputEvalTensor, nullptr);
					//updateVariables_TD(batch*BatchSize, BatchSize);
				}
			}
			//if (DBSize%BatchSize != 0) //last batch
			//{
			//	printf("______LAST_______\n");
			//	for (uint64_t i = 0; i < DBSize%BatchSize; i++)
			//	{
			//		size_t id = DBSize - DBSize%BatchSize + i;
			//		memcpy(&InputTensor(i * 8 * 8 * 14), Database[id].pos.Squares.mData, sizeof(Float) * 8 * 8 * 14);
			//		//memcpy(&OutputMoveTensor(i * 2 * 64), Database[id].move.mData, sizeof(Float) * 2 * 64);
			//		OutputEvalTensor(i) = Database[id].eval;
			//	}
			//	for (int run = 0; run < num_runs; run++)
			//	{
			//		error += NetTrain->train(InputTensor, nullptr, &OutputEvalTensor);
			//		updateVariables_TD(DBSize%BatchSize);
			//	}
			//}
		}
		printf("Final error: %f, avg: %f, movecount: %d\n", error, error / (DBSize*num_epochs), CurrentPos.movelist.size());
	}
}

void Engine::learn_eval_TD_pgn(const PGNData& pgn, double time_limit)
{
	uint64_t c = 0;
	Clock timer;
	timer.Start();

	int play_count = 12;
	size_t games_filled = 0;

	Position StartPos;

	for (size_t i = 0; i < pgn.Games.size(); i++)
	{
		printf("\n-----GAME %d-----\n", i + 1);
		StartPos.setStartPos();

		for (size_t j = 0; j < pgn.Games[i].Moves.size(); j++)
		{
			CurrentPos = StartPos;

			//Make random move
			std::vector<Move> moves;
			moves.reserve(128);
			CurrentPos.generateMoves(moves);

			Move m = moves[rand() % moves.size()];
			while (!CurrentPos.isMoveLegal(m))
			{
				m = moves[rand() % moves.size()];
			}
			assert(m.isNullMove() == false);
			CurrentPos.makeMove(m);

			//Play game
			printf("subgame %d\n", games_filled+1);
			int game_end_pos = play_count;
			for (int k = 0; k < play_count; k++)
			{
				timer.Stop();
				if (timer.ElapsedSeconds() >= time_limit)
					return;

				SearchResult search = go(MODE_MOVETIME, 350, 350, 0, 0, false);
				m = search.m;

				assert(m.isNullMove() == false);

				/*Data* d = &Database[k];
				d->pos.copyFromPosition(CurrentPos);
				d->eval = LeafEval_MatOnly();
				moveToTensor(m, &d->move);*/

				uint64_t id = games_filled*play_count + k;

				pos2posNN(&InputTensor(id * 8 * 8 * 14), CurrentPos);

				if (CurrentPos.Turn == COLOR_BLACK)
					search.eval = -search.eval;
				OutputEvalTensor(id) = search.eval/100.0;

				CurrentPos.makeMove(m);
				int status = CurrentPos.getGameStatus();
				if (status != STATUS_NOTOVER || CurrentPos.movelist.size() > 100)
				{
					game_end_pos = k+1;
					break;
				}
			}

			//Apply TD
			Float gamma = 0.5;
			for (size_t k = games_filled*play_count; k < games_filled*play_count + game_end_pos - 1; k++)
			{
				Float current_gamma = gamma;
				Float sum = 0.0;
				for (size_t l = k; l < games_filled*play_count + game_end_pos - 1; l++)
				{
					//printf("%d %d %f %f %f\n", i, j, Database[j + 1].eval, Database[j].eval, sum);
					sum += current_gamma*(OutputEvalTensor(l + 1) - OutputEvalTensor(l));
					current_gamma *= gamma;
				}
				printf("eval: %f, sum: %f\n", OutputEvalTensor(k), sum + OutputEvalTensor(k));
				OutputEvalTensor(k) = OutputEvalTensor(k) + sum;
			}
			games_filled++;

			//Continue game
			assert(pgn.Games[i].Moves[j].isNullMove() == false);
			StartPos.makeMove(pgn.Games[i].Moves[j]);

			//Batch is filled, start training
			if (games_filled >= BatchSize / play_count)
			{
				games_filled = 0;

				//Train
				int num_epochs = 10;
				int num_runs = 1;
				Float error = 0;
				for (int epoch = 0; epoch < num_epochs; epoch++)
				{
					error = 0;
					for (int run = 0; run < num_runs; run++)
					{
						error += NetTrain->train(InputTensor, &OutputEvalTensor, nullptr);
						//updateVariables_TD(batch*BatchSize, BatchSize);
					}
				}
				printf("Final error: %f, avg: %f, movecount: %d\n", error, error / (BatchSize*num_epochs), CurrentPos.movelist.size());
			}
		}
	}
}

inline Float sign(Float f)
{
	return (f > 0.0 ? 1.0 : (f == 0.0 ? 0.0 : -1.0));
}

void Engine::updateVariables_TD(uint64_t start, uint64_t batch_size)
{
	Float gamma = 0.0;
	Float learnin_rate = 0.0005;
	Float avg_change = 0.0;
	uint64_t count = 0;
	for (uint64_t i = start; i < start+batch_size; i++)
	{
		Float sum = 0.0;
		Float current_gamma = 1.0;
		for (uint64_t j = i; j < DBSize-1; j++)
		{
			sum += current_gamma*(Database[j+1].eval - Database[j].eval);
			current_gamma *= gamma;
		}
		printf("sum: %f, move: %s, eval: %f, diff: %f\n", sum, tensorToMove(&Database[i].move).toString(), Database[i].eval, Database[i + 1].eval - Database[i].eval);
		const Optimizer* opt = NetTrain->mBoard->mOptimizer;
		for (size_t j = 0; j < opt->Variables.size(); j++)
		{ 
			for (uint64_t k = 0; k < opt->Variables[j]->Delta.mSize; k++)
			{
				opt->Variables[j]->Data(k) += learnin_rate*(opt->Variables[j]->Delta(k))*sum;
				avg_change += abs(learnin_rate*opt->Variables[j]->Delta(k)*sum);
				count++;
			}
		}
	}
	printf("avg change: %f\n", avg_change / count);
	//NetPlay->mBoard->copy_variables(NetTrain->mBoard);
}

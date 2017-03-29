#include "Engine.h"

const int DATABASE_MAX_SIZE = 6400;
const int CONST_INF = 10000;

const int BATCH_SIZE = 64;

Engine::Engine()
	: BatchSize(BATCH_SIZE), InputTensor(make_shape(BATCH_SIZE, 8, 8, 14)), 
	OutputMoveTensor(make_shape(BATCH_SIZE, 2, 64)), OutputEvalTensor(make_shape(BATCH_SIZE, 1))
{
	Database = new Data[DATABASE_MAX_SIZE];
	DBCounter = 0;
	DBSize = 0;

	NetPlay = new Net(1);
	NetTrain = new Net(BatchSize);

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
}

Engine::~Engine()
{
	delete[] Database;
	delete Table;

	if(NetTrain)
		delete NetTrain;
	if(NetPlay)
		delete NetPlay;

	Table = NULL;
	Database = NULL;
	NetTrain = NULL;
	NetPlay = NULL;
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

			int leaf = LeafEval();

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
					for (int batch = 0; batch < 100; batch++)
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
							error += NetTrain->train(InputTensor, nullptr, &OutputEvalTensor);
							//printf("Error: %f\n", mNet->train(InputTensor, nullptr, &OutputEvalTensor));
						}
					}
					if (epoch == 99)
					{
						printf("Final error: %f, avg: %f\n", error, error/(64*100));
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

				GoReturn go = go_alphabeta(4 + (rand()%2));
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
				for (int epoch = 0; epoch < 100; epoch++)
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
							error += NetTrain->train(InputTensor, nullptr, &OutputEvalTensor);
							//printf("Error: %f\n", mNet->train(InputTensor, nullptr, &OutputEvalTensor));
						}
					}
					if (epoch == 99)
					{
						printf("Final error: %f, avg: %f\n", error, error / (BatchSize * DBSize));
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
	uint64_t c = 0;
	Clock timer;
	timer.Start();

	int epsilon = 75;

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

			//Make a move
			int r1 = rand() % 100;
			if (r1 < epsilon)
			{
				std::vector<Move> moves;
				moves.reserve(128);
				CurrentPos.generateMoves(moves);

				m = moves[rand() % moves.size()];
				assert(m.isNullMove() == false);

				eval = LeafEval_NN();
				if (CurrentPos.Turn == COLOR_BLACK)
				{
					eval = -eval;
				}
			}
			else
			{
				Bitset hash = CurrentPos.HashKey;

				GoReturn go = go_alphabeta(4 + (rand() % 2));
				m = go.m;
				eval = LeafEval_NN();
				if (CurrentPos.Turn == COLOR_BLACK)
				{
					eval = -eval;
				}
				assert(m.isNullMove() == false);
				assert(CurrentPos.HashKey == hash);
			}


			//Save position
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


			CurrentPos.makeMove(m);
			if (CurrentPos.getGameStatus() != STATUS_NOTOVER)
			{
				break;
			}
			c++;
		}

	
		//Train
		int num_epochs = 100;
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
					OutputEvalTensor(i) = Database[id].eval / 100.0;
				}
				for (int run = 0; run < num_runs; run++)
				{
					error += NetTrain->train(InputTensor, nullptr, &OutputEvalTensor);
					updateVariables_TD(BatchSize);
				}
			}
			if (DBSize%BatchSize != 0) //last batch
			{
				for (uint64_t i = 0; i < DBSize%BatchSize; i++)
				{
					size_t id = DBSize - DBSize%BatchSize + i;
					memcpy(&InputTensor(i * 8 * 8 * 14), Database[id].pos.Squares.mData, sizeof(Float) * 8 * 8 * 14);
					//memcpy(&OutputMoveTensor(i * 2 * 64), Database[id].move.mData, sizeof(Float) * 2 * 64);
					OutputEvalTensor(i) = Database[id].eval / 100.0;
				}
				for (int run = 0; run < num_runs; run++)
				{
					error += NetTrain->train(InputTensor, nullptr, &OutputEvalTensor);
					updateVariables_TD(DBSize%BatchSize);
				}
			}
		}
		printf("Final error: %f, avg: %f\n", error, error / (BatchSize * DBSize));
	}
}

void Engine::updateVariables_TD(uint64_t batch_size)
{
	for (uint64_t i = 0; i < DBSize; i++)
	{

	}
}

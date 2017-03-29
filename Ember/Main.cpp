#include "UCI.h"

#include <conio.h>

int main()
{
	srand(time(0));

	TTinit();
	attacksinit();
	magicinit();
	datainit();

	std::string path = "Data/variables_v4.bin";

#ifdef TRAINING_BUILD
	Engine e;
	printf("Loading weights\n");
	e.load_nets(path);
	printf("Starting training\n");
	
	e.learn_eval_TD(100, 3*60);
	
	printf("Saving weights\n");
	e.NetTrain->save(path);
	printf("Done\n");

	e.CurrentPos.loadFromFEN("2k2b1r/pp1r1ppp/2n1pq2/2p5/4QB2/2P2N2/P4PPP/1R2K2R b K - 2 0");
	e.CurrentPos.display(0);
	printf("%d %d\n", e.LeafEval_NN(), e.LeafEval());

	e.CurrentPos.loadFromFEN("rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1");
	e.CurrentPos.display(0);
	printf("%d %d\n", e.LeafEval_NN(), e.LeafEval());

	e.CurrentPos.loadFromFEN("rnb1kbr1/p2ppppp/1qp5/1p6/2PPNP2/8/PP3KPP/R1BQ1BNR w q - 1 8");
	e.CurrentPos.display(0);
	printf("%d %d\n", e.LeafEval_NN(), e.LeafEval());
	_getch();
#else
	UCI uci;
	uci.Ember.mNet->load(path);
	uci.run_uci();
#endif

	return 0;
}
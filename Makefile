.PHONY: BPlusTree

BPlusTree: simpletest.C ADS_set.h
	g++ -g -Wall -Wextra -O3 -pedantic-errors -std=c++11 -DETYPE=Person -DSIZE=7 simpletest.C -o BPlusTree;
	#g++ -g -Wall -Wextra -O3 -pedantic-errors -std=c++11 -DETYPE=int -DSIZE=3 simpletest.C -o BPlusTree;
	#g++ -g -Wall -Wextra -O3 -pedantic-errors -std=c++11 -DETYPE=std::string -DSIZE=5 simpletest.C -o BPlusTree;
	#g++ -g -Wall -Wextra -O3 -pedantic-errors -std=c++11 -DETYPE=Person -DSIZE=3 simpletest.C -o BPlusTree;
	#g++ -g -Wall -Wextra -O3 -pedantic-errors -std=c++11 -DETYPE=long -DSIZE=3 simpletest.C -o BPlusTree;
	#g++ -g -Wall -Wextra -O3 -pedantic-errors -std=c++11 -DETYPE=short -DSIZE=3 simpletest.C -o BPlusTree;
	#g++ -g -Wall -Wextra -O3 -pedantic-errors -std=c++11 -DETYPE=bool -DSIZE=3 simpletest.C -o BPlusTree;
	#g++ -g -Wall -Wextra -O3 -pedantic-errors -std=c++11 -DETYPE=char -DSIZE=3 simpletest.C -o BPlusTree;
	#g++ -g -Wall -Wextra -O3 -pedantic-errors -std=c++11 -DETYPE=float -DSIZE=3 simpletest.C -o BPlusTree;

run: BPlusTree
	./BPlusTree

debug: BPlusTree
	gdb ./BPlusTree
	
grind: BPlusTree
	valgrind -v --track-origins=yes --leak-check=full --show-leak-kinds=all --num-callers=20 ./BPlusTree

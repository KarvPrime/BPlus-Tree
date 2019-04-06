#ifndef ADS_SET_H
#define ADS_SET_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>

/*
		 _   __      _       _____   __       __  ________   _____    _____   _____
		| | / /     / \     |  _  \  \ \     / / |___  ___| |  _  \  |  ___| |  ___| (I know all the trees)
		| |/ /     / _ \    | |_|  >  \ \   / /     |  |    | |_|  > | |___  | |___  (They're the best)
		|   <     / /_\ \   |  _  /    \ \_/ /      |  |    |  _  /  |  ___| |  ___| (This is my tree)
		| |\ \   / /   \ \  | | \ \     \   /       |  |    | | \ \  | |___  | |___  (It's really great)
		|_| \_\ /_/     \_\ |_|  \_\     \_/        |__|    |_|  \_\ |_____| |_____| (V.3.1337)

											(I AM GROOT!)
											|/
                    ¬¹²³¼½¬{[]}\¸@ł€¶ŧ←↓→øþ¨~æſðđŋħł˝^`|»«¢„“”µ·…–
					°!"§$%&/()=?`QWERTZUIOPÜ+ASDFGHJKLÖÄ#<YXCVBNM,.-
					^1234567890ß´qwertzuiopü+asdfghjklöä#<yxcvbnm,.-
							\ \___  \ \ ||   / /
							 \___ \  \ \//  / /
					  (Meh...) | \ \  | |  / /
					        \| |  \ \ | | / /      (CHIRP!)
							   O   \ \| |/ /       |/
							  /|\  |      ||   "-<                       (DIS IS CTHULHU)
							   |   |  |  / |  \__/                             _____
							  / \  |  \    |  /                               / ° ° \
								   |   \   | /                               |       |
				  (WOOF!)          |     / |/          (IA CTHULHU FHTAGN!)  \__***__/
				 ,  |/ 	         \ | /  /  |                |/               // /  /|\
		  ~,____/_<	              \|/  \   |               \O/               || \ | /|
		   /_____/ 	               |    \  |                |                |\ / \ ||
__________/______\________________/_________\______________/_\_______________/|_\_/_/\__________________________________
								 /  / | | \  \
							   _/  /  / \  \  \_
*/

// Author: Karv

/*
Implementation notes:
- Everything should work fine by now - except merge which is being implemented right now.
- leaf = leaf, branch = branch, node = both
- To first check if an element has to be splitted should prevent some errors (inserting in the wrong node
 afterwards) and also has some speed benefits (no need to shift all elements to insert, then shift to other node).
- To first check if the merge wouldn't lead to a mergesplit should generally make it run faster, except in some
 weird edge cases.
Implementation thoughts:
- One could use a breadcrumb array with the length of the depth of the tree which has all the pointers
 from the find_leaf method. So there is no need for any parent pointers, but it would be a little bit slower
 and harder to implement as you'd either have to access everything from ADS_set or you die with static.
- To find the end of the node is a little bit tricky. Either use Mode::free to find the first free element, or
 put a pointer there and compare if both are pointing to the same element ... this implementation uses the size of
 the current node to find out which elements still belong to the node. The additional problem here is that one has
 to really watch out to set everything unneeded to nullptr and never ever miss to decrement the size and check
 everything thrice as otherwise there could be some unwanted stuff going on.
- count() should count the elements, but it is no real "count". With counting I understand that I have a given set
 and I count all the values until the end. If it is a wrong implementation and I have two same keys insteead of just
 one and I count one, I didn't count the keys. But otherwise in a correct implementation there can only be one key
 who is equal to that search. Maybe I overthink that a little bit. But it's the same functionality as the find() with
 the only exception that it returns 0 instead of end() and a size_type instead of an iterator to the element.
 WARNING: Comments could comprise of sarcasm, irony, sardonicism and/or movie quotes depending on current state of mind
*/

/*
 Main container - contains the data structure - may contain hazardous materials

 It does stuff. I wanted to comment everything and why I did what above every function, but... nope. Just nope.
 It simply does stuff. And it's probably even good at it.
 */
template <typename Key, size_t N = 25>  // set class template
class ADS_set {                         // main class
public:
	class Iterator;                             // iterator class
	using value_type = Key;                     // value type
	using key_type = Key;                       // key type
	using reference = key_type&;                // reference
	using const_reference = const key_type&;    // const reference
	using size_type = size_t;                   // size
	using difference_type = std::ptrdiff_t;     // pointer difference
	using iterator = Iterator;                  // iterator
	using const_iterator = Iterator;            // const iterator
	using key_compare = std::less<key_type>;    // B+-Tree order relation
	using key_equal = std::equal_to<key_type>;  // Hashing check if keys are equal
private:
	size_type current_size{0};                      // current size of the tree
	struct Node {                                   // a node of the tree
		key_type *element = new key_type[N*2]{};    // list of elements
		Node **child{nullptr};                      // list of children
		Node *parent{nullptr};                      // parent node
		size_type size{0};                          // length of current node
		Node *next{nullptr};                        // next node

		Node() { }                                      // pretty useless constructor
		~Node() {                                       // suicide and other comforts
			if (child != nullptr) {                     // if node is a branch
				for (size_type i{0}; i <= size; ++i)    // think about your loved ones one last time
					delete child[i];                    // make life pointless
				delete[] child;                         // really pointless
			}
			delete[] element;                           // remove own values, hopes and dreams
		}

		void insert_unchecked(const key_type &key, Node *ch = nullptr) {    // insert into this node without checking
			element[size++] = key;                                          // insert key into next free element
			if (ch != nullptr) {                                            // if there is a child being attached
				child[size] = ch;                                           // insert child into next free child
				ch->parent = this;                                          // set parent of new child
			}
		}
		key_type *insert(const key_type &key, Node *ch = nullptr) {     // insert element into node
			if (size == N*2) { return split(key, ch); }                 // if size is node size split beforehand and next node is the split off node
			size_type i{0};                                             // define incrementor
			for (; i < size; ++i) {                                     // as long as size of node is not reached
				if (key_compare{}(element[i], key)) { continue; }       // continue until element is larger than or equal to key
				size_type j{size};                                      // define decrementor as node size
				while (j > i) {                                         // while decrementor is larger than incrementor
					element[j] = element[j-1];                          // set current element to previous element
					if (child != nullptr) { child[j+1] = child[j]; }    // set current child to previous child
					--j;                                                // decrement
				}
				break;                                                  // break out when equal
			}
			element[i] = key;                                           // set key to empty element
			if (ch != nullptr) {                                        // if there is a child
				child[i+1] = ch;                                        // set child to empty child
				child[i+1]->parent = this;                              // set parent to this node
			}
			++size;                                                     // increase size of node
			return &element[i];                                         // return position of element
		}

		size_type remove(const key_type &key) {                             // exterminate annihilate destroy
			for (size_type i{0}; i < size; ++i) {                           // as long as size is not reached
				if (key_compare{}(element[i], key)) { continue; }           // continue if key is smaller than element
				if (key_equal{}(key, element[i])) {                         // if key is equal to element
					--size;                                                 // decrease size of node
					while (i < size) {                                      // while there is a right element
						element[i] = element[i+1];                          // set current element to next element
						++i;                                                // increment to next value
						if (child != nullptr) { child[i] = child[i+1]; }    // set right child to next child
					}
					if (child != nullptr) { child[i+1] = nullptr; }         // set last child to nullptr
					if (size < N && parent != nullptr) {                    // if size is under allowed minimum and not root node
						for (size_type j{0}; j <= parent->size; ++j) {      // find self in parent
							if (this == parent->child[j]) {                 // if found in parent
								merge();                                    // merge node
								break;                                      // break out
							}
						}
					}
					return 1;                                               // return 1 if an element has been deleted
				}
				else if (key_compare{}(key, element[i])) { break; }         // break out if key is larger than element
			}
			return 0;                                                       // return 0 if no element has been deleted
		}

		key_type *find(const key_type &key) const {                             // find key within node
			if (size == 0) { return nullptr; }                                  // if empty return nullptr
			/*
			size_type left{0};                                                  // define left side as first possible element
			size_type right{size-1};                                            // define right side as last possible element
			size_type center = (left+right)/2;                                  // get the middle element
			while (left != right) {                                             // as long as left is not right
				if (key_compare{}(element[center], key)) { left = center+1; }   // if element is smaller than key new left middle plus one
				else { right = center; }                                        // if element is larger or equal new right is middle element
				center = (left+right)/2;                                        // get the middle element
			}
			if (key_equal{}(key, element[center])) { return &element[center]; } // if search is equal to element return
			*/
			for (size_type i{0}; i < size; ++i) {                               // as long as element in use
				if (key_compare{}(key, element[i])) { break; }                  // if key is smaller than element
				else if (key_equal{}(key, element[i])) { return &element[i]; }  // if search is equal to element return
			}
			return nullptr;                                                     // else return nullptr
		}

		key_type *split(const key_type &key, Node *ch = nullptr) {                  // split a node when it's overpopulated
			if (parent == nullptr) {                                                // if node is root node
				Node *node = new Node;                                              // create the new node
				node->child = new Node*[N*2+1]();                                   // create new set of children
				node->child[0] = this;                                              // leftmost child is this node
				parent = node;                                                      // set parent node to node
			}
			Node *nx = new Node;                                                    // create new next node
			nx->next = next;                                                        // link previous next node to new next node
			next = nx;                                                              // link new next node as this next node
			if (child != nullptr) { nx->child = new Node*[N*2+1](); }               // if branch create children for next node
			nx->parent = parent;                                                    // node I am your father
			size_type i{N};                                                         // create incrementor with min size of node
			key_type *retval = nullptr;                                             // create return value
			if (child != nullptr) {                                                 // if not a leaf
				if (key_compare{}(key, element[N])) {                               // if key is smaller than middle element
					nx->insert_unchecked(element[N], child[N+1]);                   // insert middle element into next branch
					size_type j{N};                                                 // start at the middle element
					for (; j > 0; --j) {                                            // until the start of the branch
						if (key_compare{}(element[j-1], key)) { break; }            // if previous element is smaller than key break out
						else {                                                      // otherwise shift elements around
							element[j] = element[j-1];                              // current element is previous element
							child[j+1] = child[j];                                  // right child is left child
						}
					}
					element[j] = key;                                               // current element is key
					child[j+1] = ch;                                                // current right child is child
					ch->parent = this;                                              // child parent is this branch
					retval = &element[j];                                           // return value is address of current element
				}
				nx->child[0] = child[N+1];                                          // set leftmost child of next branch to first split-off child node
				child[N+1]->parent = nx;                                            // you'll get adopted too little one
				child[N+1] = nullptr;                                               // destroy enlightened link to first split-off child node
				++i;                                                                // increment and long live the resistance
				for(; i < N*2; ++i) {                                               // from after minimum size until the end
					nx->insert_unchecked(element[i], child[i+1]);                   // insert every key and child into next node
					child[i+1] = nullptr;                                           // reset pointer to child element
				}
			}
			else {
				for(; i < N*2; ++i) { nx->insert_unchecked(element[i]); }           // insert every key and child into next node
			}

			size = N;                                                               // set size to min size
			parent->insert(element[N], nx);                                         // insert middle element into parent and attach next node
			if (retval != nullptr) { return retval; }                               // if there is a return value return it
			if (!key_compare{}(key, element[N])) { return nx->insert(key, ch); }    // if element belongs into next node put it there
			else { return insert(key, ch); }                                        // otherwise put it into same element
		}

		void merge() {                                                                                  // merge a node with either left or right when it's underpopulated
			size_type pos{0};                                                                           // define position
			Node *left{nullptr};                                                                        // define left node
			Node *right{nullptr};                                                                       // define right node
			for (size_type j{0}; j <= parent->size; ++j) {                                              // find self in parent
				if (this == parent->child[j]) {                                                         // when found in parent
					pos = j;                                                                            // position of element
					if (j != 0) { left = parent->child[j-1]; }                                          // if there's a left node use it
					if (j != parent->size) { right = parent->child[j+1]; }                              // if there's a right node use it
					break;                                                                              // break out of loop if found
				}
			}
			if (right == nullptr && left == nullptr)                                                    // you can't merge without anything to merge
				throw std::runtime_error("merge epic fail");                                            // catch-22 fubar
			size_type max{(child != nullptr) ? N*2-1 : N*2};                                            // one additional element needed for branch
			if (right != nullptr) {                                                                     // if there's a right node
				if (size + right->size <= max) { push(pos, max, right, this); }                         // if no mergesplit happens do it
				else if (left != nullptr && size + left->size <= max) { push(pos-1, max, this, left); } // else if left is available and works without mergesplit
				else { push(pos, max, right, this); }                                                   // if everything fails push from next node
			}
			else { push(pos-1, max, this, left); }                                                      // if really everything fails push to left node
		}
		void push(const size_type pos, const size_type max, Node* pusher, Node* pushee) {               // push around elements when merging
			if (pusher->size + pushee->size <= max) {                                                   // if sizes are smaller or equal cut through with pushing
				if (child != nullptr) {                                                                 // if branch
					pushee->insert_unchecked(parent->element[pos], pusher->child[0]);                   // insert pusher with merger element
					pusher->child[0] = nullptr;                                                         // destroy bond with child
					for (size_type i{0}; i < pusher->size; ++i) {                                       // for all elements in pusher
						pushee->insert_unchecked(pusher->element[i], pusher->child[i+1]);               // cut through pushing to pushee
						pusher->child[i+1] = nullptr;                                                   // destroy bonds with children
					}
				}
				else {                                                                                  // otterwise
					for (size_type i{0}; i < pusher->size; ++i) {                                       // for all elements in pusher
						pushee->insert_unchecked(pusher->element[i]);                                   // cut through pushing to pushee
					}
				}
				pushee->next = pusher->next;                                                            // shorten the chain of leaves
				parent->remove(parent->element[pos]);                                                   // delete parent element
				delete pusher;                                                                          // no runes to your memory only an unmarked grave
			}
			else {                                                                                      // else lend element while doing some wibbly-wobbly timey-wimey stuff
				if (pusher->size > pushee->size) {                                                      // if pusher has more elements than pushee
					if (child != nullptr) {                                                             // if not a leaf
						pushee->insert_unchecked(parent->element[pos], pusher->child[0]);               // insert parent element and first child of right node into left node
						parent->element[pos] = pusher->element[0];                                      // set parent element leftmost child element
					}
					else {
						pushee->insert_unchecked(pusher->element[0]);                                   // insert first element of right node into left node
						parent->element[pos] = pusher->element[1];                                      // change parent to first element of next node
					}
					size_type i{1};                                                                     // start with the first element
					for (; i < pusher->size; ++i) {                                                     // as long as pusher size is not reached
						pusher->element[i-1] = pusher->element[i];                                      // shift elements to the left
						if (child != nullptr) { pusher->child[i-1] = pusher->child[i]; }                // shift children to the left
					}
					if (child != nullptr) {                                                             // if not a leaf
						pusher->child[i-1] = pusher->child[i];                                          // don't forget the last child
						pusher->child[pusher->size] = nullptr;                                          // kill unnecessary link to child
					}
					--pusher->size;                                                                     // decrease pusher size
				}
				else {                                                                                  // otherwise if pushee has more elements
					if (child != nullptr) {                                                             // if not a leaf
						for (size_type i{pusher->size}; i > 0; --i) {                                   // from last element of pusher
							pusher->element[i] = pusher->element[i-1];                                  // element is previous element
							pusher->child[i+1] = pusher->child[i];                                      // child is previous child
						}
						pusher->child[1] = pusher->child[0];                                            // shove leftmost child to the right too
						pusher->element[0] = parent->element[pos];                                      // first element is parent element
						pusher->child[0] = pushee->child[pushee->size];                                 // first child is last child of pushee
						pusher->child[0]->parent = pusher;                                              // set parent of new pusher child
						parent->element[pos] = pushee->element[pushee->size-1];                         // set parent element last pushee child element
						++pusher->size;                                                                 // increase size of pusher
						pushee->child[pushee->size] = nullptr;                                          // kill unnecessary link to child
					}
					else {                                                                              // otherwise if it's a leaf
						pusher->insert(pushee->element[pushee->size-1]);                                // insert last element from node with lower values
						parent->element[pos] = pusher->element[0];                                      // set parent element to first element
					}
					--pushee->size;                                                                     // decrease pushee size
				}
			}
		}
	};
	Node *root{nullptr};    // root node

	Node *find_leaf(const key_type &key) const {                                    // find the leaf in trees upon trees all the way down
		Node *node = root;                                                          // set current node to root
		while (node->child != nullptr) {                                            // while not a leaf
			/*
			size_type left{0};                                                      // define left side as first possible child
			size_type right{node->size};                                            // define right side as last possible child
			size_type center = (left+right)/2;                                      // get the middle child
			while (left != right) {                                                 // as long as left is not right
				if (key_compare{}(key, node->element[center])) { right = center; }  // if key is smaller new right is middle element
				else { left = center+1; }                                           // if key is larger or equal new left is middle element plus one
				center = (left+right)/2;                                            // get the middle element
			}
			node = node->child[center];                                             // set node to child node
			*/
			size_type i{0};                                                         // set incrementor to 0
			while (i < node->size) {                                                // as long as size is not reached
				if (key_compare{}(key, node->element[i])) { break; }                // break out if k is less than element
				++i;                                                                // increment
			}
			node = node->child[i];                                                  // set node to child node
		}
		return node;                                                                // return leaf node
	}

	key_type *insert_key(const key_type &key) {                     // insert a key while ignoring double values starting from a node
		Node *node = find_leaf(key);                                // find node to insert
		key_type *retval = node->find(key);                         // try to find key
		if (retval == nullptr) {                                    // if not found
			retval = node->insert(key);                             // insert key
			++current_size;                                         // increase tree size
			if (root->parent != nullptr) { root = root->parent; }   // set new root if tree grows in size
		}
		return retval;                                              // return position of inserted key
	}

	size_type remove_key(const key_type &key) {         // remove a key from the tree
		size_type retval = find_leaf(key)->remove(key); // get return value
		if (retval == 1) {                              // if deleted element
			--current_size;                             // decrement current size
			if (root->size == 0) {                      // if root shrinks to zero
				if (root->child != nullptr) {           // if root has leaves
					root = root->child[0];              // set new root
					root->parent->child[0] = nullptr;   // a child has no name
					delete root->parent;                // valar morghulis
					root->parent = nullptr;             // a root has no parent
				}
				else { clear(); }                       // root is dead
			}
		}
		return retval;                                  // return amount of deleted keys
	}

	key_type *find_pos(const key_type &key) const { // find and the position of an element
		return find_leaf(key)->find(key);           // find leaf and the key within
	}

	void output(Node *node, size_type depth, std::ostream &o = std::cerr) const {   // output a node
		if (node->child != nullptr) { output(node->child[0], depth+1, o); }         // output child on left hand path before anything else
		bool end = false;                                                           // boolean to check if end of the line is reached
		for (size_type i{0}; i < node->size; ++i) {                                 // loop to output self and right hand side
			o << ">";                                                               // symbol at beginning to differentiate trace vs normal output
			for (size_type j{0}; j <= depth; ++j) { o << " "; }                     // indent according to depth
			if (!end && i == node->size) { end = true; }                            // end of the line reached
			if (end) { o << "."; }                                                  // output - for dead keys
			else { o << node->element[i]; }                                         // output the key
			o << '\n';                                                              // line break after each element
			if (node->child != nullptr) { output(node->child[i+1], depth+1, o); }   // output child right hand side
		}
	}
public:
	ADS_set() { }                                                                               // standard constructor to create a root node
	ADS_set(std::initializer_list<key_type> ilist) { insert(ilist);	}                           // initializer list constructor
	template<typename InputIt> ADS_set(InputIt first, InputIt last) { insert(first, last); }    // input range constructor
	ADS_set(const ADS_set &other) {                                                             // clone constructor
		if (this != &other) {                                                                   // only clone if it's not the same
			clear();                                                                            // clear out everything
			const_iterator it = other.begin();                                                  // start from scratch
			if (other.begin() != other.end() && root == nullptr) { root = new Node; }           // if there are elements and there is no root create new root
			while (it != other.end()) { insert_key(*it++); }                                    // insert everything until the end is reached
		}
	}
	~ADS_set() { clear(); } // napalm - use with caution

	ADS_set &operator=(const ADS_set &other) {                                      // clone another tree without the help of CRISPR or Monsanto
		if (this == &other) { return *this; }                                       // don't clone if already the same
		clear();                                                                    // R.I.P. out the whole tree
		if (other.begin() != other.end() && root == nullptr) { root = new Node; }   // if there are samples but no petri dish get one
		const_iterator it = other.begin();                                          // get the starting sample into the petri dish
		while (it != other.end()) { insert_key(*it++); }                            // become the Master of Cloning
		return *this;                                                               // get clones for the clone war
	}
	ADS_set &operator=(std::initializer_list<key_type> ilist) { // restart with new parameters
		clear();                                                // in the beginning there was darkness and the darkness was without form and void
		insert(ilist);                                          // and in addition to the darkness there was also me and I moved upon the face of the darkness and I saw that I was alone
		return *this;                                           // let there be light
	}

	size_type size() const { return current_size; } // get the current size of the tree
	bool empty() const { return !current_size; }    // check if the tree is empty

	size_type count(const key_type &key) const {    // get the number of the beast
		if (root == nullptr) { return 0; }          // from nothing comes nothing
		return find_pos(key) != nullptr;            // return if element has been found
	}

	iterator find(const key_type &key) const {          // find the element
		if (root == nullptr) { return end(); }          // stop if no root is defined
		Node *leaf = find_leaf(key);                    // find leaf of key
		key_type *element = leaf->find(key);            // get address of element
		if (element != nullptr) {                       // if element is existing
			size_type pos = element - leaf->element;    // get position of element
			return const_iterator{leaf, pos};           // return const iterator
		}
		return end();                                   // else return end reached
	}

	void clear() {              // of course the whole point of a doomsday machine is lost if you keep it a secret
		if (root != nullptr) {  // only if root is existing
			delete root;        // purge the tree with thermonucular fire
			current_size = 0;   // set population to zero
			root = nullptr;     // start repopulation efforts
		}
	}

	void swap(ADS_set &other) {                 // swap two trees
		using std::swap;                        // swap
		swap(current_size, other.current_size); // swap current sizes of both trees
		swap(root, other.root);                 // swap roots of both trees
	}

	void insert(std::initializer_list<key_type> ilist) {                // insert elements from an initializer list
		if (ilist.size() != 0 && root == nullptr) { root = new Node; }  // if ilist has elements and there is no root create root node
		for (const key_type &key: ilist) {                              // for all values in the list
			insert_key(key);                                            // insert all values from ilist
		}
	}
	std::pair<iterator, bool> insert(const key_type &key) {                     // insert and get pair of iterator and boolean
		size_type size = current_size;                                          // create check for size
		if (root == nullptr) { root = new Node; }                               // create new node if node is lost in time and space
		key_type *k = insert_key(key);                                          // insert key
		Node *leaf = find_leaf(key);                                            // find leaf of key
		size_type pos = k - leaf->element;                                      // find position of element
		return std::make_pair(const_iterator{leaf, pos}, size != current_size); // check size and return pair
	}
	template<typename InputIt> void insert(InputIt first, InputIt last) {   // insert a certain range of elements
		if (first != last && root == nullptr) { root = new Node; }          // if there are elements and there is no root create root
		while (first != last) { insert_key(*first++); }                     // while first element is not last insert and increment
	}

	size_type erase(const key_type &key) {  // erase an element
		if (root == nullptr) { return 0; }  // return 0 if no root
		return remove_key(key);             // remove the key
	}

	const_iterator begin() const {                                  // get iterator on first element
		if (root == nullptr) { return const_iterator{}; }           // return null iterator if no root
		Node *node = root;                                          // select root
		while (node->child != nullptr) { node = node->child[0]; }   // find leftmost element
		return const_iterator{node, 0};                             // return iterator
	}
	const_iterator end() const { return const_iterator{}; }         // get iterator end outside of tree

	void dump(std::ostream &o = std::cerr) const {      // dump all variables to outputstream
		if (root != nullptr) { output(root, 0, o); }    // output data structure
		else o << "Not initialized.";                   // output error message
	}

	friend bool operator==(const ADS_set &lhs, const ADS_set &rhs) {            // compare two trees
		if (lhs.root == nullptr && rhs.root == nullptr) { return true; }        // true if both are not existing
		else if (lhs.root == nullptr || rhs.root == nullptr) { return false; }  // false if only one is not existing
		if (lhs.current_size != rhs.current_size) { return false; }             // false if sizes differ
		Node *node = lhs.root;                                                  // get root of left hand tree
		while(node->child != nullptr) { node = node->child[0]; }                // go to the leftmost leaf
		do {
			for (size_type i{0}; i < node->size; ++i) {                         // for every element in the leaf
				if (!rhs.find_pos(node->element[i])) { return false; }          // if a value isn't in rhs trees are different
				if (i == node->size) { break; }                                 // break out if end of node is reached
			}
		} while ((node = node->next) != nullptr);                               // as long as the last leaf isn't reached
		return true;                                                            // otherwise return true
	}
	friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs) { return !(lhs == rhs); }    // check if two trees are inequal
};

template <typename Key, size_t N>   // iterator class template
class ADS_set<Key,N>::Iterator {    // iterator class
	Node *nodepos{nullptr};         // current node
	size_type keypos{0};            // position of element
public:
	using value_type = Key;                                 // value type
	using difference_type = std::ptrdiff_t;                 // pointer difference
	using reference = const value_type&;                    // reference
	using pointer = const value_type*;                      // pointer
	using iterator_category = std::forward_iterator_tag;    // forward iterator

	explicit Iterator(Node *leaf = nullptr, size_type pos = 0) {    // standard constructor to create an iterator
		if (!leaf) { return; }                                      // return current iterator if no leaf is set
		nodepos = leaf;                                             // set position of node to leaf
		keypos = pos;                                               // set position of key to pos
	}
	reference operator*() const { return nodepos->element[keypos]; }    // return value of element
	pointer operator->() const { return &nodepos->element[keypos]; }    // return address of element
	Iterator &operator++() {                // prefix operator
		if (++keypos == nodepos->size) {    // if incremented position of key is size of node
			nodepos = nodepos->next;        // set position of node to next node
			keypos = 0;                     // set position of key to 0
		}
		return *this;                       // return iterator object
	}
	Iterator operator++(int) {  // postfix operator
		Iterator temp{*this};   // save iterator object in helper varible
		++*this;                // increment iterator
		return temp;            // return iterator object from helper variable
	}
	friend bool operator==(const Iterator &lhs, const Iterator &rhs) {  // check if iterators are equal
		return lhs.nodepos == rhs.nodepos && lhs.keypos == rhs.keypos;  // return equality check
	}
	friend bool operator!=(const Iterator &lhs, const Iterator &rhs) { return !(lhs == rhs); }  // check if iterators are different
};

template <typename Key, size_t N> void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); }    // swap two trees

#endif // ADS_SET_H
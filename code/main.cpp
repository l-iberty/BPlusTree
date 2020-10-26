#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <cassert>
#include <ctime>
#include <cstdlib>
#include <Windows.h>
#include <gtest\gtest.h>
#include "b_plus_tree.h"

using namespace std;

struct IntComparator {
  int operator()(const std::pair<int, int>& lhs,
    const std::pair<int, int>& rhs) const {
    if (lhs.first == rhs.first) {
      return 0;
    }
    return (lhs.first > rhs.first) ? 1 : -1;
  }
};

TEST(BPlusTree, KeyComparatorTest) {
  IntComparator cmp;
  ASSERT_EQ(0, cmp(std::make_pair(10, 0), std::make_pair(10, 10)));
  ASSERT_EQ(-1, cmp(std::make_pair(5, 0), std::make_pair(7, 0)));
  ASSERT_EQ(1, cmp(std::make_pair(12, 0), std::make_pair(10, 10)));
}

TEST(BPlusTree, SimpleTest1) {
  BPlusTree<int, int, IntComparator> tree(5);
  ASSERT_NE(nullptr, tree.Insert(26, 6));
  ASSERT_NE(nullptr, tree.Insert(19, 9));
  ASSERT_NE(nullptr, tree.Insert(10, 0));
  ASSERT_NE(nullptr, tree.Insert(37, 7));
  ASSERT_NE(nullptr, tree.Insert(3, 3));
  ASSERT_NE(nullptr, tree.Insert(6, 6));
  ASSERT_NE(nullptr, tree.Insert(8, 8));
  ASSERT_NE(nullptr, tree.Insert(18, 8));
  ASSERT_NE(nullptr, tree.Insert(13, 3));
  ASSERT_NE(nullptr, tree.Insert(25, 5));
  ASSERT_NE(nullptr, tree.Insert(23, 3));
  ASSERT_NE(nullptr, tree.Insert(29, 9));
  ASSERT_NE(nullptr, tree.Insert(30, 0));
  ASSERT_NE(nullptr, tree.Insert(38, 8));
  ASSERT_NE(nullptr, tree.Insert(35, 5));
  ASSERT_NE(nullptr, tree.Insert(40, 0));
  ASSERT_NE(nullptr, tree.Insert(31, 1));
  ASSERT_NE(nullptr, tree.Insert(51, 1));
  ASSERT_NE(nullptr, tree.Insert(55, 5));

  std::cerr << "dumping B+Tree to dot files \"test1_1.dot\", \"test1_2\"...\n";
  tree.DumpToDot("test1_1.dot", "test1_2.dot");
}

TEST(BPlusTree, SimpleTest2) {
  BPlusTree<int, int, IntComparator> tree(3);
  ASSERT_NE(nullptr, tree.Insert(26, 6));
  ASSERT_NE(nullptr, tree.Insert(19, 9));
  ASSERT_NE(nullptr, tree.Insert(10, 0));
  ASSERT_NE(nullptr, tree.Insert(37, 7));
  ASSERT_NE(nullptr, tree.Insert(3, 3));
  ASSERT_NE(nullptr, tree.Insert(6, 6));
  ASSERT_NE(nullptr, tree.Insert(8, 8));
  ASSERT_NE(nullptr, tree.Insert(18, 8));
  ASSERT_NE(nullptr, tree.Insert(13, 3));
  ASSERT_NE(nullptr, tree.Insert(25, 5));
  ASSERT_NE(nullptr, tree.Insert(23, 3));
  ASSERT_NE(nullptr, tree.Insert(29, 9));
  ASSERT_NE(nullptr, tree.Insert(30, 0));
  ASSERT_NE(nullptr, tree.Insert(38, 8));
  ASSERT_NE(nullptr, tree.Insert(35, 5));
  ASSERT_NE(nullptr, tree.Insert(40, 0));
  ASSERT_NE(nullptr, tree.Insert(31, 1));
  ASSERT_NE(nullptr, tree.Insert(51, 1));
  ASSERT_NE(nullptr, tree.Insert(55, 5));

  std::cerr << "dumping B+Tree to dot files \"test2_1.dot\", \"test2_2\"...\n";
  tree.DumpToDot("test2_1.dot", "test2_2.dot");
}

TEST(BPlusTree, SimpleTest3) {
  BPlusTree<int, int, IntComparator> tree(5);
  std::vector<std::pair<int, int>> kva;
  BPlusTree<int, int, IntComparator>::Node *node = nullptr;

  /* insert and lookup */
  int value;
  srand(static_cast<unsigned int>(time(nullptr)));
  for (int i = 0; i < 1000; i++) {
    int x = rand();
    kva.push_back(std::make_pair(x, x));
    node = tree.Insert(x, x);
    ASSERT_NE(node, nullptr);
    node = tree.Lookup(x, value);
    ASSERT_NE(node, nullptr);
    ASSERT_EQ(value, x);
  }
  std::cerr << "number of kv inserted: " << kva.size() << "\n";
  /* lookup again */
  for (size_t i = 0; i < kva.size(); ++i) {
    node = tree.Lookup(kva[i].first, value);
    ASSERT_NE(node, nullptr);
    ASSERT_EQ(value, kva[i].second);
  }

  std::cerr << "dumping B+Tree to dot files \"test3_1.dot\", \"test3_2\"...\n";
  tree.DumpToDot("test3_1.dot", "test3_2.dot");
}

TEST(BPlusTree, Compare1) {
  std::vector<int> vec = std::vector<int>(static_cast<size_t>(10e4));
  std::map<int, int> stlmap;
  BPlusTree<int, int, IntComparator> bptree = BPlusTree<int, int, IntComparator>(5);
  clock_t start, end;

  srand(static_cast<unsigned int>(time(nullptr)));
  for (size_t i = 0; i < vec.size(); ++i) {
    vec[i] = rand();
  }

  std::cerr << "stl map INSERT test begins...\n";
  start = clock();
  for (size_t i = 0; i < vec.size(); ++i) {
    stlmap[vec[i]] = vec[i];
  }
  end = clock();
  std::cerr << "stl map INSERT test ends: " << static_cast<double>((end - start)) / CLOCKS_PER_SEC << "\n";

  std::cerr << "B+Tree INSERT test begins...\n";
  start = clock();
  for (size_t i = 0; i < vec.size(); ++i) {
    bptree.Insert(vec[i], vec[i]);
  }
  end = clock();
  std::cerr << "B+Tree INSERT test ends: " << static_cast<double>((end - start)) / CLOCKS_PER_SEC << "\n";

  std::cerr << "stl map FIND test begins...\n";
  start = clock();
  for (size_t i = 0; i < vec.size(); ++i) {
    auto it = stlmap.find(vec[i]);
    ASSERT_NE(it, stlmap.end());
    ASSERT_EQ(it->second, vec[i]);
  }
  end = clock();
  std::cerr << "stl map FIND test ends: " << static_cast<double>((end - start)) / CLOCKS_PER_SEC << "\n";

  std::cerr << "B+Tree FIND test begins...\n";
  start = clock();
  for (size_t i = 0; i < vec.size(); ++i) {
    int v;
    ASSERT_NE(nullptr, bptree.Lookup(vec[i], v));
    ASSERT_EQ(v, vec[i]);
  }
  end = clock();
  std::cerr << "B+Tree FIND test ends: " << static_cast<double>((end - start)) / CLOCKS_PER_SEC << "\n";
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
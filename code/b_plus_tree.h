#ifndef B_PLUS_TREE_H
#define B_PLUS_TREE_H

#include <algorithm>
#include <cassert>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#define SEP "#!~#"

char *COLORS[] = { "IndianRed", "PaleGreen" };

template <class ForwardIt, class T, class Comparator>
ForwardIt LowerBound(ForwardIt first, ForwardIt last, const T &value, Comparator comp) {
  ForwardIt it;
  typename std::iterator_traits<ForwardIt>::difference_type count, step;
  count = std::distance(first, last);

  while (count > 0) {
    it = first;
    step = count / 2;
    std::advance(it, step);
    if (comp(*it, value) < 0) {
      first = ++it;
      count -= step + 1;
    } else {
      count = step;
    }
  }
  return first;
}

template <class Key, class Value, class Comparator>
class BPlusTree {
 public:
  enum NodeType { InternalNode = 0, LeafNode, InvalidNode };

  struct Node {
    NodeType type;
    Node *parent;
    std::vector<std::pair<Key, Value>> kva;
    std::vector<Node *> ptrs;

    Node(NodeType _type) : type(_type), parent(nullptr) {}
    std::string ToString() const;
  };

  BPlusTree(int n) : n_(n), node_(nullptr), mem_blk_(new char[kMemBlkSize]) {
    if (n < 3) {
      throw std::invalid_argument("n must be greater than 2");
    }
  }
  ~BPlusTree() {
    delete node_;
    delete[] mem_blk_;
  }
  Node *Insert(const Key &K, const Value &V);
  Node *Lookup(const Key &K, Value &V);
  void DumpToDot(const char *filename1, const char *filename2) const;

 private:
  Node *node_;

  // n_ 是 B+Tree 的阶, 就是常说的"M 阶 B+Tree"、"M-way B+Tree"的"M",
  // 是一个节点的子节点数目的最大值.
  // Every inner node other than the root is at least half full (M/2 − 1 <= num of keys <= M − 1), M/2 向上取整
  // 基于这条规则可知, M 的最小值为3. 有说法认为 M=2 时就是二叉树, 但此时 (0 <= number of keys <= 1), 如果允许节点内
  // 没有 key 就会发生一些很奇怪的情形, 所以规定 M 最小值为3
  size_t n_;

  char *mem_blk_;
  Comparator comp_;
  enum { kMemBlkSize = 0x1000 };

  Node *FindLeafNodeShouldContainKey(const Key &K) const;
  Node *InsertInLeaf(Node *L, const Key &K, const Value &V, Node* P);
  Node *InsertInParent(Node *N, const Key &K, Node *N1);
  void SetParentForChildren(Node *N);
};

template <class Key, class Value, class Comparator>
std::string BPlusTree<Key, Value, Comparator>::Node::ToString() const {
  std::string s = "\"";
  std::string tag = (type == LeafNode) ? "*" : "";
  int i;
  for (i = 0; i < static_cast<int>(kva.size()) - 1; ++i) {
    s += std::to_string(kva[i].first) + tag + " ";
  }
  if (i == kva.size() - 1) {
    s += std::to_string(kva[i].first) + tag;
  } else if (kva.empty()) {
    s += "NIL";
  }
  s += "\"";
  return s;
}

template <class Key, class Value, class Comparator>
typename BPlusTree<Key, Value, Comparator>::Node *
BPlusTree<Key, Value, Comparator>::Insert(const Key &K, const Value &V) {
  if (node_ == nullptr) { /* tree is empty */
    node_ = new Node(LeafNode);
    node_->kva.push_back(std::make_pair(K, V));
    node_->ptrs.assign(2, nullptr);
    return node_;
  }
  Node *L = FindLeafNodeShouldContainKey(K);
  assert(L != nullptr);
  auto ite = LowerBound(L->kva.begin(), L->kva.end(), std::make_pair(K, Value()), comp_);
  if (ite != L->kva.end() && ite->second == V) {
    return L; /* key-value is already exists */
  }
  if (L->kva.size() < n_ - 1) {
    InsertInLeaf(L, K, V, nullptr);
    return L;
  }

  /* L has n-1 keys already, split it */
  /* Create Node L' */
  Node *L1 = new Node(LeafNode);

  /* Copy L.P1 ... L.Kn-1 to a block of memory T that can hold n (pointer, key-value) pairs */
  Node *T = new (mem_blk_) Node(InvalidNode);
  size_t n = L->kva.size();
  T->kva.insert(T->kva.begin(), L->kva.begin(), L->kva.begin() + n);
  T->ptrs.insert(T->ptrs.begin(), L->ptrs.begin(), L->ptrs.begin() + n);
  InsertInLeaf(T, K, V, nullptr);

  /* Erase L.P1 through L.Kn-1 from L */
  L->ptrs.erase(L->ptrs.begin(), --L->ptrs.end());
  L->kva.erase(L->kva.begin(), L->kva.end());

  /* Set L'.Pn = L.Pn; Set L.Pn = L' */
  L1->ptrs.insert(L1->ptrs.begin(), L->ptrs.back());
  *(L->ptrs.begin()) = L1;

  /* Copy T.P1 through T.K(n/2) from T into L starting at L.P1 */
  /* Copy T.P(n/2)+1 through T.Kn from T into L' starting at L'.P1 */
  size_t j = (n_ + 1) / 2;
  L->ptrs.insert(L->ptrs.begin(), T->ptrs.begin(), T->ptrs.begin() + j);
  L->kva.insert(L->kva.begin(), T->kva.begin(), T->kva.begin() + j);
  L1->ptrs.insert(L1->ptrs.begin(), T->ptrs.begin() + j, T->ptrs.end());
  L1->kva.insert(L1->kva.begin(), T->kva.begin() + j, T->kva.end());

  /* K' = L'->kva.front().first */
  return InsertInParent(L, L1->kva.front().first, L1);
}

template <class Key, class Value, class Comparator>
typename BPlusTree<Key, Value, Comparator>::Node *
BPlusTree<Key, Value, Comparator>::Lookup(const Key &K, Value &V) {
  if (node_ == nullptr) {
    return nullptr;
  }
  Node *L = FindLeafNodeShouldContainKey(K);
  if (L == nullptr) {
    return nullptr;
  }
  auto ite = LowerBound(L->kva.begin(), L->kva.end(), std::make_pair(K, Value()), comp_);
  if (ite != L->kva.end() && ite->first == K) {
    V = ite->second;
    return L;
  }
  return nullptr;
}

template <class Key, class Value, class Comparator>
void BPlusTree<Key, Value, Comparator>::DumpToDot(const char *filename1, const char *filename2) const {
  std::ofstream dot1, dot2;
  std::string s = "digraph G {\n"; /* 按 B+Tree 的一般表示方法绘制 */
  std::string backtraces = "digraph G {\n"; /* 绘制每个节点通过 parent 指针回溯到根节点的路径 */

  if (node_ != nullptr) {
    std::string attrs;
    std::map<std::string, int> visited1, visited2;
    std::map<int, std::vector<Node *>> levelNodes;
    std::queue<Node *> q;
    Node *node = node_;
    int level = 0;
    q.push(node);
    while (!q.empty()) {
      level++;
      size_t n = q.size();
      while (n-- > 0) {
        node = q.front();
        q.pop();
        /* save node by according to its level */
        if (levelNodes.find(level) == levelNodes.end()) {
          levelNodes[level] = std::vector<Node *>({node});
        } else {
          levelNodes[level].push_back(node);
        }
        /* attrs */
        std::string attr = "\t" + node->ToString() + "[ style=filled, fillcolor=" + std::string(COLORS[node->type]) + " ];\n";
        if (std::string::npos == attrs.find(attr)) {
          attrs += attr;
        }
        /* backtrace */
        std::string sznode = node->ToString();
        if (node->parent != nullptr) {
          std::string szparent = node->parent->ToString();
          std::string k = sznode + SEP + szparent;
          if (visited2.find(k) == visited2.end()) {
            visited2[k] = int();
            backtraces += "\t" + sznode + " -> " + szparent + ";\n";
          }
        }
        /* forwarding */
        for (size_t i = 0; i < node->ptrs.size(); ++i) {
          Node *temp = node->ptrs[i];
          if (temp != nullptr) {
            q.push(temp);
            std::string sztemp = temp->ToString();
            std::string k = sznode + SEP + sztemp;
            if (visited1.find(k) == visited1.end()) {
              visited1[k] = int();
              s += "\t" + sznode + " -> " + sztemp + ";\n";
            }
          }
        }
      }
    }
    std::string rank;
    for (auto it = levelNodes.begin(); it != levelNodes.end(); ++it) {
      std::vector<Node *> &nodes = it->second;
      rank += "\t{ rank=same ";
      for (size_t i = 0; i < nodes.size(); ++i) {
        rank += nodes[i]->ToString() + " ";
      }
      rank += "};\n";
    }
    s += rank + attrs;
    backtraces += rank + attrs;
  }
  s += "}";
  backtraces += "}";
  dot1.open(filename1);
  dot2.open(filename2);
  dot1 << s;
  dot2 << backtraces;
  dot1.close();
  dot2.close();
}

template <class Key, class Value, class Comparator>
typename BPlusTree<Key, Value, Comparator>::Node *
BPlusTree<Key, Value, Comparator>::FindLeafNodeShouldContainKey(const Key &K) const {
  // Assumes no duplicate keys, and returns pointer to the record with
  // search key K if such a record exists, and null otherwise.
  if (node_ == nullptr) {
    return nullptr;
  }
  Node *node = node_;
  while (node->type != LeafNode) {
    auto ite = LowerBound(node->kva.begin(), node->kva.end(), std::make_pair(K, Value()), comp_);
    size_t i = ite - node->kva.begin();
    if (ite == node->kva.end()) {
      /* Find last non-null pointer in the node */
      auto rite = node->ptrs.rbegin();
      while (rite != node->ptrs.rend() && *rite == nullptr) {
        rite++;
      }
      if (rite != node->ptrs.rend()) {
        node = *rite;
      }
    } else if (K == node->kva[i].first) {
      node = node->ptrs[i + 1];
    } else { /* K < node->kva[i].first */
      node = node->ptrs[i];
    }
  }
  /* node->type == LeafNode */
  return node;
}

template <class Key, class Value, class Comparator>
typename BPlusTree<Key, Value, Comparator>::Node *
BPlusTree<Key, Value, Comparator>::InsertInLeaf(Node *L, const Key &K, const Value &V, Node* P) {
  auto ite = LowerBound(L->kva.begin(), L->kva.end(),
      std::make_pair(K, Value()), Comparator());
  size_t i = ite - L->kva.begin();
  if (ite == L->kva.end()) {
    L->kva.push_back(std::make_pair(K, V));
  } else {
    L->kva.insert(ite, std::make_pair(K, V));
  }
  L->ptrs.insert(L->ptrs.begin() + i, P);
  return L;
}

template <class Key, class Value, class Comparator>
typename BPlusTree<Key, Value, Comparator>::Node *
BPlusTree<Key, Value, Comparator>::InsertInParent(Node *N, const Key &K1, Node *N1) {
  if (N->parent == nullptr) { /* N is the root of the tree */
    /* Create a new node R containing N,K',N' */
    /* Make R the root of the tree */
    node_ = new Node(InternalNode);
    node_->kva.push_back(std::make_pair(K1, Value()));
    node_->ptrs.push_back(N);
    node_->ptrs.push_back(N1);
    SetParentForChildren(node_);
    return node_;
  }
  Node *P = N->parent;
  assert(P != nullptr);
  if (P->ptrs.size() < n_) {
    /* insert (K',N') in P just after N */
    auto ite = std::find(P->ptrs.begin(), P->ptrs.end(), N);
    assert(ite != P->ptrs.end());
    size_t i = ite - P->ptrs.begin();
    if (i == P->ptrs.size() - 1) {
      P->kva.push_back(std::make_pair(K1, Value()));
      P->ptrs.push_back(N1);
    } else {
      P->kva.insert(P->kva.begin() + i, std::make_pair(K1, Value()));
      P->ptrs.insert(P->ptrs.begin() + i + 1, N1);
    }
    SetParentForChildren(P);
    return P;
  }

  /* Split P */
  /* Copy P to a block of memory T that can hold P and (K',N') */
  Node *T = new (mem_blk_) Node(InvalidNode);
  T->kva.assign(P->kva.begin(), P->kva.end());
  T->ptrs.assign(P->ptrs.begin(), P->ptrs.end());

  /* Insert (K',N') into T just after N */
  auto ite = std::find(T->ptrs.begin(), T->ptrs.end(), N);
  assert(ite != T->ptrs.end());
  size_t i = ite - T->ptrs.begin();
  T->ptrs.insert(ite + 1, N1);
  T->kva.insert(T->kva.begin() + i, std::make_pair(K1, Value()));

  size_t pivot = (n_ + 1 + 1) / 2;
  if (i < pivot) {
    /* 如果 N1 被插入到前半部分, 那么分裂后 N1->parent 应该指向 P */
    N1->parent = P;
  }

  /* Erase all entries from P; Create node P' */
  P->kva.erase(P->kva.begin(), P->kva.end());
  P->ptrs.erase(P->ptrs.begin(), P->ptrs.end());
  Node *P1 = new Node(InternalNode);

  /* Copy T.P' ... T.P((n+1)/2) into P */
  i = pivot;
  P->kva.assign(T->kva.begin(), T->kva.begin() + i - 1);
  P->ptrs.assign(T->ptrs.begin(), T->ptrs.begin() + i);
  /* K'' = T.K((n+1)/2) */
  Key K2 = T->kva[i - 1].first;
  /* Copy T.P((n+1)/2+1) ... T.P(n+1) into P' */
  P1->kva.assign(T->kva.begin() + i, T->kva.end());
  P1->ptrs.assign(T->ptrs.begin() + i, T->ptrs.end());

  SetParentForChildren(P1);
  return InsertInParent(P, K2, P1);
}

template <class Key, class Value, class Comparator>
void BPlusTree<Key, Value, Comparator>::SetParentForChildren(Node *N) {
  if (N == nullptr) {
    return;
  }
  for (size_t i = 0; i < N->ptrs.size(); ++i) {
    if (N->ptrs[i] != nullptr) {
      N->ptrs[i]->parent = N;
    }
  }
}

#endif /* B_PLUS_TREE_H */

#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <string>
#include <functional>

extern "C"{
#include "avl.h"
#include "bst.h"
#include "rb.h"
}

using namespace std;

int string_compare(const void *a, const void *b, void *p)
{
  const string &sa = *(const string *) a;
  const string &sb = *(const string *) b;

  if(sa < sb) return -1;
  else if(sa > sb) return 1;
  else return 0;
}

template<class Node, void*(Node::*nodeData), Node*(Node::*nodeLink)[2]>
void preorderTraverse(const Node *node)
{
  if(node == nullptr) return ;
  cout << *(string*)(node->*nodeData) << ' ';
  if((node->*nodeLink)[0] != nullptr || (node->*nodeLink)[1] != nullptr){
    cout << "(";
    preorderTraverse<Node, nodeData, nodeLink>((node->*nodeLink)[0]);
    cout << ", ";
    preorderTraverse<Node, nodeData, nodeLink>((node->*nodeLink)[1]);
    cout << ")";
  }
}

int main(int argc, char *argv[])
{
  avl_table *avlTree = avl_create(string_compare, NULL, NULL);
  bst_table *bstTree = bst_create(string_compare, NULL, NULL);
  rb_table *rbTree = rb_create(string_compare, NULL, NULL);

  string str;
  //while(getline(cin, str)){
  for(int i = 0; i < 32; ++i){
    getline(cin, str);
    avl_probe(avlTree, new string(str));
    bst_probe(bstTree, new string(str));
    rb_probe(rbTree, new string(str));
  }

  preorderTraverse<avl_node, &avl_node::avl_data, &avl_node::avl_link>(avlTree->avl_root);
  cout << endl;
  preorderTraverse<bst_node, &bst_node::bst_data, &bst_node::bst_link>(bstTree->bst_root);
  cout << endl;
  preorderTraverse<rb_node, &rb_node::rb_data, &rb_node::rb_link>(rbTree->rb_root);
  cout << endl;
  
  return 0;
}


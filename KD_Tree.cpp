#include <vector>
#include <iomanip>
#include <algorithm>
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <stack>
#include <pthread.h>
using namespace std;

class Node {
public:
  Node* left;
  Node* right;
  int index;
  int dimension;
  Node (int index ,int dimension) {
    this->index = index;
    this->dimension = dimension;
  }
};

struct Parameters{
  vector<vector<double> >* data;
  int begin;
  int end;
  int dimension;
};

void printData(const vector<vector<double> >& data) {
  cout << endl;
  for (vector<double> v: data) {
    for (double d: v) {
      cout << left <<setfill(' ') << setw(10) << left << d;
    }
    cout << endl;
  }
  cout << endl;
}

bool compareFunc(const vector<double>& first, const vector<double>& second) {
  assert(first.back() == second.back());
  int dim_index = first.back();
  return first[dim_index] < second[dim_index];
}

void localSort(vector<vector<double> >& v, int begin, int end, int dimension) {
  for (int i = begin; i <= end; ++i) {
    v[i].back() = dimension;
  }
  sort(v.begin()+begin, v.begin()+end+1, compareFunc);
}

void localSort_1(struct Parameters * param) {
  vector<vector<double> >* d = param->data;
  int begin = param->begin;
  int end = param->end;
  int dimension = param->dimension;
  for (int i = begin; i <= end; ++i) {
    d->at(i).back() = dimension;
  }
  sort(d->begin()+begin, d->begin()+end+1, compareFunc);
  int mid = (begin+end)/2;
  pthread_t pid_1;
  struct Parameters * p_1 = (struct Parameters *)malloc(sizeof(struct Parameters));
  p_1->data = d;
  p_1->begin = begin;
  p_1->end = mid-1;
  p_1->dimension = dimension+1;
  pthread_t pid_2;
  struct Parameters * p_2 = (struct Parameters *)malloc(sizeof(struct Parameters));
  p_2->data = d;
  p_2->begin = mid+1;
  p_2->end = end;
  p_2->dimension = dimension+1;
  int s_1 = pthread_create(&pid_1, NULL, &localSort_1, (void*)p_1);
  int s_2 = pthread_create(&pid_2, NULL, &localSort_1, (void*)p_2);
  pthread_join(&pid_1, NULL);
  pthread_join(&pid_2, NULL);
  free(p_1);
  free(p_2);
  return;
}

void prepareTreeHelper(vector<vector<double> >& data, int begin, int end, int dimension) {
  if (begin >= end) return;
  localSort(data, begin, end, dimension);
  int mid = (begin+end)/2;
  prepareTreeHelper(data, begin, mid-1, dimension+1);
  prepareTreeHelper(data, mid+1, end, dimension+1);
  return;
}

void prepareTree(vector<vector<double> >& data) {
  for (int i = 0; i < data.size(); ++i) {
    data[i].push_back(0);
  }
  prepareTreeHelper(data, 0, data.size()-1, 0);
  for (int i = 0; i < data.size(); ++i) {
    data[i].pop_back();
  }
  return;
}

Node* buildTreeHelper(const vector<vector<double> >& data,int begin, int end, int dimension) {
  if (begin > end) return NULL;
  if (begin == end) {
    return new Node(begin, dimension);
  }
  int mid = (begin+end)/2;
  Node* cur = new Node(mid, dimension);
  cur->left = buildTreeHelper(data, begin, mid-1, dimension+1);
  cur->right = buildTreeHelper(data, mid+1, end, dimension+1);
  return cur;
}

Node* buildTree(vector<vector<double> >& data) {
  prepareTree(data);
  Node* root = buildTreeHelper(data, 0, data.size()-1,0);
  return root;
}

void printTree(const vector<vector<double> >& data, Node* root) {
  if (root == NULL) return;
  printTree(data, root->left);
  for (int i = 0; i < root->dimension; ++i) {
    cout << "      ";
  }
  cout << data[root->index][root->dimension%data[root->index].size()] << endl;
  printTree(data, root->right);
  return;
}

vector<vector<double> > randomData(int size_of_data, int dimension) {
  vector<vector<double> > v(size_of_data, vector<double>(dimension, 1));
  for (int i = 0; i < v.size(); ++i) {
    for (int j = 0; j < v[i].size(); ++j) {
      v[i][j] = (double)(rand()%20);
    }
  }
  return v;
}

double euclideanMetric(const vector<double>& first, const vector<double>& second) {
  assert(first.size() == second.size());
  double distance = 0;
  for (int i = 0; i < first.size(); ++i ) {
    distance += first[i]*first[i] + second[i]*second[i];
  }
  return sqrt(distance);
}

void query(const vector<vector<double> >& data, Node* current, const vector<double>& p, vector<pair<double, Node*> >& knn) {
  if (current == NULL) return;
  double r = knn.back().first;
  double d = euclideanMetric(p, data[current->index]);
  if (d < r) {
    knn.back().first = d;
    knn.back().second = current;
    sort(knn.begin(), knn.end());
  }
  r = knn.back().first;
  double cuttingLine = data[current->index][current->dimension];
  bool first_check_left = false;
  if (p[current->dimension] <= cuttingLine) {
    // on the left
    query(data, current->left, p, knn);
    first_check_left = true;
  } else {
    query(data, current->right, p, knn);
  }

  double d_cross_dim = abs(cuttingLine-p[current->dimension]);
  if (first_check_left) {
    if (d_cross_dim < knn.back().first) {
      query(data, current->right, p, knn);
    }
  } else {
    if (d_cross_dim < knn.back().first) {
      query(data, current->left, p, knn);
    }
  }
  return;
}


//int main(int argc, char const *argv[]) {
//  srand(time(NULL));
//  vector<vector<double> > data = randomData(20, 5);
//  Node* root = buildTree(data);
//  printData(data);
//  printTree(data, root);
//  return 0;
//}

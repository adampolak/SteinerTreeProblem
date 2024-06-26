#include <signal.h>
#include "SteinerTree.h"
#include "utils.h"
using namespace std;

int currentTol;
long long Globalbest = 1e16;
SteinerTree bestI;
int generation = 0;
bool volatile finished = false;
SteinerTreeProblem* SteinerTree::SteinerTreeproblem;
DSU dsu; 
long long maxThreshold;
bool operator< (const edge& a, const edge& b){
	if(a.w < b.w) return true;
	else if(a.w > b.w) return false;
	else return make_pair(a.u, a.v) < make_pair(b.u, b.v);
}

///////////////////// Funciones de un union find disjoints sets ///////////////////////////////
DSU::DSU(int n){
	p.resize(n + 1);
	for(int i = 0; i <= n; i++) p[i] = i;
}

int DSU::getS(int u){
	return (p[u] == u ? u : (p[u] = getS(p[u])));
}

bool DSU::sameSet(int u, int v){
	used.insert(u);
	used.insert(v);
	return getS(u) == getS(v);
}

void DSU::join(int u, int v){
	used.insert(u);
	used.insert(v);
	u = getS(u);
	v = getS(v);
	if(u != v) p[u] = v;
}

void DSU::reset(){
	FOREACH(u, used) p[*u] = *u;
	used.clear();
}


//////////////////////////// Funciones del individuo ////////////////////////////////

// Calcula el fitness "por fuerza bruta"
long long SteinerTree::calculateFitnessComplete(){
	long long F = 0;
	dsu = DSU(SteinerTreeproblem->n);
	FOREACH(e, SteinerTreeproblem->edges) if(I[e->u] && I[e->v]){
		if(!dsu.sameSet(e->u, e->v)){
			dsu.join(e->u, e->v);
			F += e->w;
		}
	}
	dsu.reset();
	return F;
}

bool SteinerTree::calculateFitness2(){
	fitness = 0;
	int cnt[SteinerTreeproblem->n];
	bzero(cnt, sizeof(cnt));
	vector<pair<int, long long> > st[SteinerTreeproblem->n];
	
	int c = 0;
	FOREACH(e, edges){
		if(!dsu.sameSet(e->u, e->v)){
			dsu.join(e->u, e->v);
			cnt[e->u]++, cnt[e->v]++;
			fitness += e->w;
			st[e->u].push_back(make_pair(e->v, e->w));
			st[e->v].push_back(make_pair(e->u, e->w));
			c++;
		}
	}
	queue<int> q;
	FOREACH(v, dsu.used) if(cnt[*v] == 1 && !(SteinerTreeproblem->fixed)[*v]) q.push(*v);
	while(!q.empty()){
		int u = q.front(); q.pop();
		I[u] = false;
		cnt[u] = 0;
		FOREACH(v, st[u]) if(I[v->first]){
			cnt[v->first]--;
			fitness -= v->second;
			if(cnt[v->first] == 1 && !(SteinerTreeproblem->fixed)[v->first]) q.push(v->first);
		}
	}
	dsu.reset();

	if(fitness < Globalbest){
		sigset_t sign;
		sigemptyset(&sign);
		sigaddset(&sign, SIGTERM);
		sigprocmask(SIG_BLOCK, &sign, NULL);
		bestI.I.clear();
		for(int u = 0; u < (SteinerTreeproblem->n); u++){
			if((SteinerTreeproblem->fixed)[u] || cnt[u] >= 1){
				bestI.I.push_back(true);
			} else {
				bestI.I.push_back(false);
			}
		}
		Globalbest = fitness;
		sigprocmask(SIG_UNBLOCK, &sign, NULL);
		cerr << "value " << Globalbest << endl;
	}
	return true;
}

bool SteinerTree::calculateFitness(){
	evaluateMinDistances();
	return true;
}

// Inserta el nodo u al individuo
void SteinerTree::insert(int u){
}

void SteinerTree::insert(vector<int> us){
}


// Elimina el nodo u del individuo
void SteinerTree::erase(int u){
}

void printBest(){
	unordered_map<int, vector<pair<int, long long> > > mst;
	bestI.getMST(mst);
  bestI.print(mst);
}


void SteinerTree::reset(){
	edges.clear();
	for(int u = 0; u < (SteinerTreeproblem->n); u++)
		if(I[u]) FOREACH(v, (SteinerTreeproblem->adj)[u]) if(v->first < u && I[v->first]) edges.push_back(edge(v->first, u, v->second));
	sort(edges.begin(), edges.end());
	calculateFitness2();
}

SteinerTreeProblem::SteinerTreeProblem(){
	string s;
	cin >> s;
	cin >> s;
	cin >> s;
	cin >> n;
	cin >> s;
	cin >> m;
	int u, v;
	long long w;
	adj.resize(n);
	fixed.resize(n);
	for(int i = 0; i < m; i++){
		cin >> s >> u >> v >> w; u--, v--;
		edges.push_back(edge(u, v, w));
		adj[u].push_back(make_pair(v, w));
		adj[v].push_back(make_pair(u, w));
	}
	sort(edges.begin(), edges.end());
	maxThreshold = edges[edges.size() / 2].w * 2;
	cin >> s;
	cin >> s;
	cin >> s;
	cin >> s;
	int T; cin >> T;
	for(int i = 0; i < T; i++){
		cin >> s >> u; u--;
		fixed[u] = true;
		fs.push_back(u);
	}
}

vector<int> p, sz;
int mx;

void init(vector<bool> &I){
	p.resize(I.size());
	sz.resize(I.size());
	int len = p.size();
	for(int i = 0; i < len; i++) p[i] = i;
	for(int i = 0; i < len; i++) sz[i] = (I[i] ? 1 : 0);
}

int getS(int u){
	if(p[u] == u) return u;
	else return p[u] = getS(p[u]);
}

bool sameS(int u, int v){
	return getS(u) == getS(v);
}

void join(int u, int v){
	u = getS(u), v = getS(v);
	if(u == v) return;
	p[u] = v;
	sz[v] += sz[u];
	mx = max(mx, sz[v]);
}



bool first = true;
void SteinerTree::restart(){
	I.resize(SteinerTreeproblem->n, false);
	IComplete.resize(SteinerTreeproblem->n, false);
	for(int i = 0; i < (int)(SteinerTreeproblem->fs).size(); i++)
		I[(SteinerTreeproblem->fs)[i]] = true;
	for(int i = 0; i < (SteinerTreeproblem->n); i++) 
		if(rand()/(RAND_MAX + 1.0) < 0.15) I[i] = true;
	dsu = DSU((int)I.size());
	calculateFitness();
	return;
}

void SteinerTree::addCrossover(SteinerTree &ind2){
}

void SteinerTree::uniformCrossover(SteinerTree &ind){
	for(int i = 0; i < (SteinerTreeproblem->n); i++)
		if(rand()%2 == 0) swap(I[i], (ind.I)[i]);
	calculateFitness();
	ind.calculateFitness();
}

int SteinerTree::getDistance(SteinerTree &ind){
	int ans = 0;
	for(int i = 0; i < (SteinerTreeproblem->n); i++)
		if(I[i] != (ind.I)[i]) ans++;
	return ans;
}

void SteinerTree::pathMutation(int k){
}

void SteinerTree::uniformMutation(double pm){
	long long prevFitness = fitness;
	bool changed = false;
	vector<bool> prevI = I;
	vector<bool> prevIComplete = IComplete;
	for(int i = 0; i < (SteinerTreeproblem->n); i++) if(!(SteinerTreeproblem->fixed)[i])
		if(rand()/(RAND_MAX + 1.0) < pm){
			changed = true;
			I[i] = !I[i];
		}
	if (changed){
		calculateFitness();
		if (prevFitness < fitness){
			I = prevI;
			IComplete = prevIComplete;
			fitness = prevFitness;
		}
	}
}

void SteinerTree::dependentMutation(double pm){
	int limit = (generation < 100)?0:5;
	for (int i = 0; i < limit; i++){
		if (SteinerTreeproblem->n - SteinerTreeproblem->fs.size() != 0){
			uniformMutation(2.0 / (SteinerTreeproblem->n - SteinerTreeproblem->fs.size()));
		}
	}
}

void SteinerTree::dependentCrossover(SteinerTree &ind){
	uniformCrossover(ind);
}

// Guarda en mst el minimum spanning tree del individuo
void SteinerTree::getMST(unordered_map<int, vector<pair<int, long long> > > &mst){
	edges.clear();
	for(int u = 0; u < (SteinerTreeproblem->n); u++) if(I[u]){
		FOREACH(v, (SteinerTreeproblem->adj)[u]) if(u < v->first && I[v->first]) edges.push_back(edge(u, v->first, v->second));
	}
	sort(edges.begin(), edges.end());
	mst.clear();
	FOREACH(e, edges){
		if(!dsu.sameSet(e->u, e->v)){
			dsu.join(e->u, e->v);
			mst[e->u].push_back(make_pair(e->v, e->w));
			mst[e->v].push_back(make_pair(e->u, e->w));
		}
	}
	dsu.reset();
}

void SteinerTree::print(unordered_map<int, vector<pair<int, long long> > > &mst){
    unordered_map<int, vector<pair<int, long long> > >::iterator it;

    long long cost = 0;
    for(it = mst.begin(); it != mst.end(); it ++)
    	FOREACH(v, it->second) cost += (v->second);
    
    cost = cost/2;

    cout << "VALUE " << cost << endl;

    for(it = mst.begin(); it != mst.end(); it ++)
    	FOREACH(v, it->second)
        if(it->first < v->first)
            cout << it->first + 1 << " " << v->first + 1 << endl;
}

vector<int> hsh_r;

long long counter = 0;

set<pair<long long, int> > order;

void SteinerTreeProblem::dijkstra(vector<int> &u, vector<long long> &dist, vector<int> &p, vector<bool> &tree, priority_queue<pair<long long, int>, vector<pair<long long, int> >, greater<pair<long long, int> > > &pq, vector<bool> &I){
	if(p.empty()){
		long long INF = 1e15;
		p = vector<int>(n);
		dist = vector<long long>(n, INF);
		tree = vector<bool>(n, false);
	}

	for (int i = 0; i < u.size(); i++){
		tree[u[i]] = true;
		if(order.count(make_pair(dist[u[i]], u[i])) == 1) order.erase(make_pair(dist[u[i]], u[i])); 
		// Actualizamos order
		dist[u[i]] = 0; pq.push(make_pair(dist[u[i]], u[i]));
	}

	bool found = false;
	long long value = -1e15;
	while(!pq.empty()){
		pair<long long, int> tmp = pq.top(); pq.pop();
		long long d = tmp.first;
		int n = tmp.second;
		if(dist[n] != d) continue;
		FOREACH(vw, adj[n]){
			counter++;
			int v = vw->first;
			long long w = vw->second;
			if(!tree[v]){
				if(dist[v] > dist[n] + w){
					// Actualizamos order si se encontro algo mejor de algunos de los que aun no se han alcanzado
					if(I[v] && !tree[v]){
						if(order.count(make_pair(dist[v], v)) == 1) order.erase(make_pair(dist[v], v));
						order.insert(make_pair(dist[n] + w, v));
					}
					dist[v] = dist[n] + w;
					p[v] = n;
					pq.push(make_pair(dist[v], v));
					hsh_r[v] = 1;
				}
				else if(dist[v] == dist[n] + w && w != 0){
					if(rand()/(RAND_MAX + 1.0) < 1.0/hsh_r[v]) p[v] = n;
					hsh_r[v]++;
				}
			}
		}
		if (I[n] && (!tree[n])){
			if(!found) value = dist[n], found = true;
		}
		if(found && dist[n] > value + currentTol) return;
	}
}

long long fitnessBest = 1e15;

void SteinerTree::evaluateMinDistances(){
	long long best = 1e15;
	hsh_r = vector<int>(SteinerTreeproblem->n, 1);
	vector<bool> Ibest, ICompletebest;
	int cnt_break = 0;
	currentTol = rand() % 2;
	if (currentTol != 0){
		currentTol = 1 + rand() % (maxThreshold + 1);
	}

	int selectionType = random() % 2;

	while(true){
		order.clear();
		int u = (SteinerTreeproblem->fs)[rand()%((int)(SteinerTreeproblem->fs).size())];
		fitness = 0;
		vector<long long> dist;
		vector<int> p;
		vector<bool> tree;
		vector<int> nodes;
		nodes.push_back(u);
		priority_queue<pair<long long, int>, vector<pair<long long, int> >, greater<pair<long long, int> > > pq;
		SteinerTreeproblem->dijkstra(nodes, dist, p, tree, pq, I);
		vector<int> cnt(SteinerTreeproblem->n, 0);
		int aux = 0;
		int connected = 1;

		while(true){
     if (connected == SteinerTreeproblem->fs.size()){
        for (int i = 0; i < SteinerTreeproblem->n; i++){
          if ((I[i]) && (!tree[i])){
            if (SteinerTreeproblem->fixed[i]){
              cerr << "Error interno" << endl;
            }
            I[i] = false;
          }
        }
				break;
      }   

			aux++;
			long long mn = 1e15;
			u = -1;

			int cnt_r = 1;
			mn = order.begin()->first;
			long long t;
			FOREACH(v, order){
				if(v->first > mn + currentTol) break;
				if(rand()/(RAND_MAX + 1.0) < 1.0/cnt_r) u = v->second, t = v->first;
				cnt_r++;
			}

			if(u == -1) break;
			if (selectionType == 1){//Greedy
				u = -1;
				cnt_r = 1;
				FOREACH(v, order){
        	if(v->first > mn + currentTol) break;
        	if ((u != -1) && (!SteinerTreeproblem->fixed[u]) && (SteinerTreeproblem->fixed[v->second])){
          	cnt_r = 1;
          	u = v->second;
          	continue;
        	}
        	if ((u != -1) && (SteinerTreeproblem->fixed[u]) && (!SteinerTreeproblem->fixed[v->second])) continue;
        	if(rand()/(RAND_MAX + 1.0) < 1.0/cnt_r) u = v->second, t = v->first;
        	cnt_r++;
      	}

				if (!SteinerTreeproblem->fixed[u]){
 	       vector<int> nodes;
 	       FOREACH(v, order){
 	         if(v->first > mn + currentTol) break;
 	         nodes.push_back(v->second);
 	       }
 	       random_shuffle(nodes.begin(), nodes.end());
 	       vector < pair < int, pair<int, int > > > scoreAndNode;
 	       for (int i = 0; i < min((int)nodes.size(), 50); i++){
 	         int node = nodes[i];
 	         int score = 0;
 	         for (int i = 0; i < (SteinerTreeproblem->adj)[node].size(); i++){
 	           int otherNode = (SteinerTreeproblem->adj)[node][i].first;
 	           if ((SteinerTreeproblem->fixed[otherNode]) && ((!tree[otherNode]))){
 	             score++;
 	           }
 	         }
 	         scoreAndNode.push_back(make_pair(score, make_pair(random() % 100000, node)));
 	       }
 	       sort(scoreAndNode.begin(), scoreAndNode.end());
 	       u = scoreAndNode.back().second.second;
 	 	    }
			}

			if ((u != -1) && (SteinerTreeproblem->fixed[u])){
        connected++;
      }   

			order.erase(make_pair(dist[u], u)); // Eliminamos el ya utilizado
			fitness += dist[u];
			vector<int> path;
			while(!tree[u]){
				path.push_back(u);
				cnt[u]++, cnt[p[u]]++;
				u = p[u];
			}
			reverse(path.begin(), path.end());
			SteinerTreeproblem->dijkstra(path, dist, p, tree, pq, I);
			if (finished){
				printBest();
				exit(0);
			}
		}
		vector<bool> copyI = I;
		vector<bool> otherI(copyI.size());
		for(int u = 0; u < (SteinerTreeproblem->n); u++){
			if((SteinerTreeproblem->fixed)[u] || cnt[u] >= 1){
				I[u] = true;
			} else {
				I[u] = false;
			}
		}
		reset();
		I = copyI;

		if(best > fitness){
			best = fitness;
			Ibest = I;
			ICompletebest = IComplete;
			cnt_break = 0;
			if(fitness < Globalbest){
				cerr << "Error interno" << endl;
			}
		}
		else cnt_break++;
		if (cnt_break > 3) break; 
		int ones = 0;
		for(int u = 0; u < (SteinerTreeproblem->n); u++){
			if ((!(SteinerTreeproblem->fixed)[u]) && (I[u])){
				ones++;
			}
		}
		int equals = 0;
		//if (typeTransform == 0){
			for(int u = 0; u < (SteinerTreeproblem->n); u++){
				if((SteinerTreeproblem->fixed)[u] || cnt[u] >= 3){
					if ((!(SteinerTreeproblem->fixed)[u]) && (I[u])) equals++;
					I[u] = true; 
				} else {
					I[u] = false;
				}
			}
		//}
	}
	I = Ibest;
	IComplete = ICompletebest;
	fitness = best;
}

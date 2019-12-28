#include<iostream>
#include<ctime>
using namespace std;

enum goodsstate { ship, crane, buffer, shuttle, warehouse };
enum shipstate { pre, ing, post };
enum poptype { pmax, pmin };

class Cgoods
{
public:
	int time;
	int shipno;
	int goodsno;
	int lifeline[3];							//入港、起吊、运送时间点
	int warehouseno;
	goodsstate state;//货物状态

};

class Cship
{
public:
	int shipno;
	Cgoods sgoods[6];
	int intime, outtime;
	int goodsqtt;
	int lifeline[2];							//入港、出港时间点
	shipstate state;//货轮状态
};

template<class T>
struct cmp_fgoods
{
	bool operator() (T x, T y)
	{
		return x.time > y.time;					//小的优先级高
	}
};

int shuttletime[5];
template<class T>
struct cmp_bgoods
{
	bool operator() (T x, T y)
	{
		return shuttletime[x.warehouseno] > shuttletime[y.warehouseno];		//小的优先级高
	}
};

template<class T>
struct cmp_ship
{
	bool operator() (T x, T y)
	{
		return x.intime + x.outtime > y.intime + y.outtime;			//小的优先级高
	}
};


template<class T>
struct Node //结点
{		
	T data;
	Node* next;
};

template<class T, typename fcmp>
class Cqueue				//自定义序列
{
private:
	Node<T>* head;			//头指针，data为空
	int size;
public:
	Cqueue ();
	~Cqueue ();
	void push (T a);
	T min ();
	T max ();
	bool empty ();
	void pop (poptype t);
	void swap (int i, int j);
};

template<class T, typename fcmp>
Cqueue<T, fcmp>::Cqueue ()
{
	head = new Node<T>;
	head->next = NULL;              //建立带头结点的链表
	size = 0;
}

//删除结点，释放内存
template<class T, typename fcmp>
Cqueue<T, fcmp>::~Cqueue ()
{
	Node<T>* p;		//临时结点指针,保存要释放的结点
	size = 0;
	while (head)
	{
		p = head;
		head = head->next;        //通过让p保存当前要释放的结点,然后让head往下移动，再释放p指向的空间,循环如此达到完全释放
		delete p;
	}
}

template<class T, typename fcmp>
void Cqueue<T, fcmp>::push (T a)
{
	Node<T>* p = head;
	Node<T>* s = new Node<T>;
	fcmp cmp;
	s->data = a;
	s->next = NULL;
	if (size == 0)
		head->next = s;
	else
	{
		while (p->next != NULL)
		{
			bool go = cmp (a, p->next->data);
			if (go)
				p = p->next;		//定位到i-1的位置
			else
				break;
		}
		s->next = p->next;
		p->next = s;
	}
	size++;
}

template<class T, typename fcmp>
T Cqueue<T, fcmp>::min ()//队列开始
{
	T s ;
	s = head->next->data;
	return s;
}

template<class T, typename fcmp>
T Cqueue<T, fcmp>::max ()//队列末尾
{
	Node<T>* p = head;
	while (p->next!=NULL)
	{
		p = p->next;		
	}
	T s;
	s = p->data;
	return s;
}

template<class T, typename fcmp>
bool Cqueue<T, fcmp>::empty ()
{
	if (size == 0)
		return true;
	else
		return false;
}

template<class T, typename fcmp>
void Cqueue<T, fcmp>::pop (poptype t)
{
	if (t == pmax)
	{
		Node<T>* p = head;
		while (p->next->next != NULL)
		{
			p = p->next;
		}
		Node<T>* s = p->next;     //保存要释放的结点
		p->next = NULL;
		delete s;
		size--;
	}
	else
	{
		Node<T>* p = head->next;
		head->next = p->next;
		delete p;
		size--;
	}
}

template<class T, typename fcmp>
void Cqueue<T, fcmp>::swap (int i, int j)
{
	//分别找到待交换结点的前驱
	Node<T>* node1, * node2;
	Node<T>* prenode1, * prenode2;
	Node<T>* p = head;
	int i0 = 0;
	while (i0 < i || i0 < j)
	{
		if (i0 == i - 1)
		{
			prenode1 = p;
			node1 = p->next;
		}
		if (i0 == j - 1)
		{
			prenode2 = p;
			node2 = p->next;
		}
		i0++;
		p = p->next;
	}
	//特殊情况处理
	if (i + 1 == j)		//结点1在结点2的前面
	{
		Node<T>* postnode2 = node2->next;
		prenode1->next = node2;
		node2->next = node1;
		node1->next = postnode2;
		return;
	}
	if (j + 1 == i)
	{
		Node<T>* postnode1 = node2->next;
		prenode2->next = node1;
		node1->next = node2;
		node2->next = postnode1;
		return;
	}
	Node<T>* postnode1 = node2->next;
	Node<T>* postnode2 = node2->next;
	//一般情况直接交换两个结点
	prenode1->next = node2;
	node2->next = postnode1;
	prenode2->next = node1;
	node1->next = postnode2;
}

int totaltime;
int shipqtt, berthqtt, craneqtt, shuttleqtt, warehouseqtt;		//货轮、泊位、吊车、小车、仓库数量
Cgoods craninggoods[5], shuttlinggoods[5];
Cship parkingship[5];
int freeberth, freecrane, freeshuttle;
Cqueue<Cgoods,  cmp_fgoods<Cgoods>> freegoods;					//待起吊货物
Cqueue<Cgoods,  cmp_bgoods<Cgoods>> buffergoods;				//缓冲区货物
Cqueue<Cship,  cmp_ship<Cship>> freeship;						//待泊货轮

int main ()
{
	clock_t start, finish;
	double runtime;
	cout << "贪心算法2:" << endl;
	cout << "泊位：由小到大，起吊：由小到大，运送：由大到小" << endl;
	cout << "货轮、泊位、吊车、小车、仓库数量" << endl;
	cin >> shipqtt >> berthqtt >> craneqtt >> shuttleqtt >> warehouseqtt;
	memset (shuttletime, 0, 5);
	for (int i = 0; i < warehouseqtt; i++)
	{
		cout << "仓库" << i << "时间" << endl;
		cin >> shuttletime[i];
	}
	for (int i = 0; i < shipqtt; i++)							//输入条件
	{
		Cship temp;
		cout << "货轮" << i + 1 << "进、出时间" << endl;
		temp.shipno = i;
		cin >> temp.intime >> temp.outtime;						//驶入、驶出时间
		int goodsqtt;
		cout << "货物数" << endl;
		cin >> goodsqtt;										//货物数量
		temp.goodsqtt = goodsqtt;
		temp.state = pre;
		memset (temp.lifeline, 0, 2);
		cout << goodsqtt << "次起吊时间和仓库编号" << endl;
		for (int j = 0; j < goodsqtt; j++)
		{
			cin >> temp.sgoods[j].time >> temp.sgoods[j].warehouseno;	//每件货物所需时间
			temp.sgoods[j].shipno = i;
			temp.sgoods[j].goodsno = j;
			temp.sgoods[j].state = ship;
			memset (temp.sgoods[j].lifeline, 0, 3);
		}
		freeship.push (temp);
	}
	start = clock ();//开始时间
	freeberth = berthqtt;
	freecrane = craneqtt;
	freeshuttle = shuttleqtt;
	while (freeberth < berthqtt || freecrane < craneqtt || freeshuttle < shuttleqtt || !buffergoods.empty () || !freegoods.empty () || !freeship.empty ())		//当无货物时停止
	{
		/*泊位*/
		/*在泊*/
	OUT:
		for (int i = 0; i < berthqtt - freeberth; i++)
		{
			if (parkingship[i].lifeline[0] + parkingship[i].intime == totaltime)					//入泊完成
			{
				parkingship[i].state = ing;//从parkingship移至freegoods
				for (int j = 0; j < parkingship[i].goodsqtt; j++)
					freegoods.push (parkingship[i].sgoods[j]);
			}
			if (!parkingship[i].goodsqtt && parkingship[i].state == ing)							//装卸完成
			{
				parkingship[i].state = post;
				parkingship[i].lifeline[1] = totaltime;
			}
			if (parkingship[i].lifeline[1] + parkingship[i].outtime == totaltime)					//出泊完成
			{
				freeberth++;//从parkingship移出
				for (int j = i; j < berthqtt - freeberth; j++)
					parkingship[j] = parkingship[j + 1];
				i--;
			}
		}
		/*入泊*/
		while (freeberth && !freeship.empty ())
		{
			Cship temp = freeship.min ();//从freeship队列移至parkingship
			temp.lifeline[0] = totaltime;
			for (int i = 0; i < temp.goodsqtt; i++)
				temp.sgoods[i].lifeline[0] = totaltime;
			parkingship[berthqtt - freeberth] = temp;
			freeship.pop (pmin);
			freeberth--;
		}
		/*起吊机*/
		/*在吊*/
		for (int i = 0; i < craneqtt - freecrane; i++)
		{
			if (craninggoods[i].lifeline[1] + craninggoods[i].time == totaltime)		//起吊完成
			{
				craninggoods[i].state = buffer;//从craninggoods移出
				buffergoods.push (craninggoods[i]);
				int shipno = craninggoods[i].shipno;
				int goodsno = craninggoods[i].goodsno;
				freecrane++;
				for (int j = i; j < craneqtt - freecrane; j++)
					craninggoods[j] = craninggoods[j + 1];
				bool found = false;									
				for (int j = 0; j < berthqtt - freeberth && !found; j++)//删除货轮上货物
				{
					if (parkingship[j].shipno == shipno)
					{
						for (int k = 0; k < parkingship[j].goodsqtt && !found; k++)
						{
							if (parkingship[j].sgoods[k].goodsno == goodsno)
							{
								found = true;
								parkingship[j].goodsqtt--;
								for (int l = k; l < parkingship[j].goodsqtt; l++)
									parkingship[j].sgoods[l] = parkingship[j].sgoods[l + 1];
								if (parkingship[j].goodsqtt == 0)
									goto OUT;
							}
						}
					}
				}
				i--;
			}
		}
		/*起吊*/
		while (freecrane && !freegoods.empty ())//从freegoods队列移至craninggoods
		{
			Cgoods temp = freegoods.min ();
			temp.lifeline[1] = totaltime;
			temp.state = crane;
			craninggoods[craneqtt - freecrane] = temp;
			freegoods.pop (pmin);
			freecrane--;
		}
		/*小车*/
		/*在运*/
		for (int i = 0; i < shuttleqtt - freeshuttle; i++)
		{
			if (shuttlinggoods[i].lifeline[2] + 2 * shuttletime[shuttlinggoods[i].warehouseno] == totaltime)	//运输完成
			{
				shuttlinggoods[i].state = warehouse;//从shuttlinggoods移出
				freeshuttle++;
				for (int j = i; j < shuttleqtt - freeshuttle; j++)
					shuttlinggoods[j] = shuttlinggoods[j + 1];
				i--;
			}
		}
		/*起运*/
		while (freeshuttle && !buffergoods.empty ())//从buffergoods队列移至shuttlinggoods
		{
			Cgoods temp = buffergoods.max ();
			temp.lifeline[2] = totaltime;
			temp.state = shuttle;
			shuttlinggoods[shuttleqtt - freeshuttle] = temp;
			buffergoods.pop (pmax);
			freeshuttle--;
		}
		totaltime++;
	}
	cout << totaltime - 1;
	finish = clock ();//结束计时
	runtime = (double)(finish - start);
	cout << "运行时间为" << runtime << "ms" << endl;//运行时间
	return 0;
}
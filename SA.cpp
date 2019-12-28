#include<iostream>
#include<queue>
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
	int cw[2];//起吊优先级，当前优先级和较优优先级
	int sw[2];//运送优先级，当前优先级和较优优先级
	goodsstate state;//货物状态

};

class Cship
{
public:
	int shipno;
	Cgoods sgoods[6];
	int intime, outtime;
	int goodsqtt;
	int bw[2];			//入泊优先级，当前优先级和较优优先级					
	int lifeline[2];							//入港、出港时间点
	shipstate state;//货轮状态
};

template<class T>
struct cmp_fgoods
{
	bool operator() (T x, T y, int i)
	{//默认是less函数
		return x.cw[i] >= y.cw[i];					//小的优先级高
	}
};

int shuttletime[5];
template<class T>
struct cmp_bgoods
{
	bool operator() (T x, T y, int i)
	{//默认是less函数
		return x.sw[i] >= y.sw[i];		//小的优先级高
	}
};

template<class T>
struct cmp_ship
{
	bool operator() (T x, T y, int i)
	{//默认是less函数
		return x.bw[i] >= y.bw[i];			//小的优先级高
	}
};

template<class T>
struct Node //结点
{
	T data;
	Node* next;
};

template<class T, typename fcmp>
class Cqueue				//从小到大
{
private:
	Node<T>* head;			//头指针
	int size;
public:
	Cqueue ();	
	~Cqueue ();
	void push (T a, int i);
	T min ();
	T max ();
	bool empty ();
	void pop (poptype p);
	void copy (Cqueue& Q,int i);
	void setbw (int shipno, int bw);
	void setcw (int shipno, int goodsno, int cw);
	void setsw (int shipno, int goodsno, int sw);
	void bswap (int m, int n, int i);//交换入泊优先级
	void cswap (int m, int n, int i);//交换起吊优先级
	void sswap (int m, int n, int i);//交换运送优先级
	void save (int i);
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
void Cqueue<T, fcmp>::push (T a, int i)
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
			bool go = cmp (a, p->next->data, i);
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
	T s;
	s = head->next->data;
	return s;
}

template<class T, typename fcmp>
T Cqueue<T, fcmp>::max ()//队列末尾
{
	Node<T>* p = head;
	while (p->next != NULL)
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
void Cqueue<T, fcmp>::copy (Cqueue& Q,int i)
{
	while (!this->empty ())
		this->pop (pmin);
	Node<T>* p = Q.head;
	while (p->next != NULL)
	{
		p = p->next;
		T s = p->data;
		this->push (s, i);
	}
}

template<class T, typename fcmp>
void Cqueue<T, fcmp>::setbw (int shipno, int bw)//初始化入泊优先级
{
	Node<T>* p = this->head;
	while (p->next != NULL)
	{
		p = p->next;
		if (p->data.shipno == shipno)
		{
			p->data.bw[0] = bw;
			p->data.bw[1] = bw;
		}
	}
}

template<class T, typename fcmp>
void Cqueue<T, fcmp>::setcw (int shipno, int goodsno, int cw)//初始化起吊优先级
{
	Node<T>* p = this->head;
	while (p->next != NULL)
	{
		p = p->next;
		if (p->data.shipno == shipno)
		{
			for (int j = 0; j < p->data.goodsqtt; j++)
			{
				if (p->data.sgoods[j].goodsno == goodsno)
				{
					p->data.sgoods[j].cw[0] = cw;
					p->data.sgoods[j].cw[1] = cw;
				}
			}
		}
	}
}

template<class T, typename fcmp>
void Cqueue<T, fcmp>::setsw (int shipno, int goodsno, int sw)//初始化运送优先级
{
	Node<T>* p = this->head;
	while (p->next != NULL)
	{
		p = p->next;
		if (p->data.shipno == shipno)
		{
			for (int j = 0; j < p->data.goodsqtt; j++)
			{
				if (p->data.sgoods[j].goodsno == goodsno)
				{
					p->data.sgoods[j].sw[0] = sw;
					p->data.sgoods[j].sw[1] = sw;
				}
			}
		}
	}
}

template<class T, typename fcmp>
void Cqueue<T, fcmp>::bswap (int m, int n, int i)//交换入泊优先级
{
	Node<T>* p = this->head;
	while (p->next != NULL)
	{
		p = p->next;
		if (p->data.bw[i] == m)
			p->data.bw[i] = n;
		else if (p->data.bw[i] == n)
			p->data.bw[i] = m;
	}
}

template<class T, typename fcmp>
void Cqueue<T, fcmp>::cswap (int m, int n, int i)//交换起吊优先级
{
	Node<T>* p = this->head;
	while (p->next != NULL)
	{
		p = p->next;
		for (int j = 0; j < p->data.goodsqtt; j++)
		{
			if (p->data.sgoods[j].cw[i] == m)
				p->data.sgoods[j].cw[i] = n;
			else if (p->data.sgoods[j].cw[i] == n)
				p->data.sgoods[j].cw[i] = m;
		}
	}
}

template<class T, typename fcmp>
void Cqueue<T, fcmp>::sswap (int m, int n, int i)//交换运送优先级
{
	Node<T>* p = this->head;
	while (p->next != NULL)
	{
		p = p->next;
		for (int j = 0; j < p->data.goodsqtt; j++)
		{
			if (p->data.sgoods[j].sw[i] == m)
				p->data.sgoods[j].sw[i] = n;
			else if (p->data.sgoods[j].sw[i] == n)
				p->data.sgoods[j].sw[i] = m;
		}
	}
}

template<class T, typename fcmp>
void Cqueue<T, fcmp>::save (int i)
{
	Node<T>* p = this->head;
	while (p->next != NULL)
	{
		p = p->next;
		p->data.bw[1 - i] = p->data.bw[i];
		for (int j = 0; j < p->data.goodsqtt; j++)
		{
			p->data.sgoods[j].cw[1 - i] = p->data.sgoods[j].cw[i];
			p->data.sgoods[j].sw[1 - i] = p->data.sgoods[j].sw[i];
		}
	}
}

int totaltime;
int shipqtt, berthqtt, craneqtt, shuttleqtt, warehouseqtt;		//货轮、泊位、吊车、小车、仓库数量
int totalgoods;
Cgoods craninggoods[5], shuttlinggoods[5];
Cship parkingship[5];
int freeberth, freecrane, freeshuttle;
int F0;
int ans;
Cqueue<Cgoods, cmp_fgoods<Cgoods>> freegoods[5];				//待起吊货物
Cqueue<Cship, cmp_ship<Cship>> freeship0;						//待泊货轮
Cqueue<Cship, cmp_ship<Cship>> freeship;						//待泊货轮
Cqueue<Cgoods, cmp_bgoods<Cgoods>> buffergoods;					//缓冲区货物

int main ()
{
	clock_t start, finish;
	double runtime;
	F0 = 0;
	ans = 0;
	totalgoods = 0;
	cout << "SA模拟退火" << endl;
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
		temp.bw[0] = temp.intime + temp.outtime;
		int goodsqtt;
		cout << "货物数" << endl;
		cin >> goodsqtt;										//货物数量
		totalgoods += goodsqtt;
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
			temp.sgoods[j].cw[0] = temp.sgoods[j].time;
			temp.sgoods[j].sw[0] = 0;
			memset (temp.sgoods[j].lifeline, 0, 3);
		}
		freeship0.push (temp, 0);
	}
	start = clock ();
	int iw = 0;
	int bw0 = 0, cw0 = 18, sw0 = 0;
	double t = 10000;																						// 温度 
	while (t > 0.0000000000000001)
	{
		//计算
		freeship.copy (freeship0, iw);																		//初始化
		int inship = 0, outship = 0;
		totaltime = 0;
		freeberth = berthqtt;
		freecrane = craneqtt;
		freeshuttle = shuttleqtt;
		while (freeberth < berthqtt || freeshuttle < shuttleqtt || !buffergoods.empty () || !freeship.empty ())		//当无货物时停止
		{
			/*泊位*/
			/*在泊*/
		OUT:
			for (int i = 0; i < berthqtt - freeberth; i++)
			{
				if (parkingship[i].lifeline[0] + parkingship[i].intime == totaltime)					//入泊完成
				{
					parkingship[i].state = ing;
					for (int j = 0; j < parkingship[i].goodsqtt; j++)
						freegoods[inship].push (parkingship[i].sgoods[j], iw);
					inship++;
				}
				if (!parkingship[i].goodsqtt && parkingship[i].state == ing)							//装卸完成
				{
					parkingship[i].state = post;
					parkingship[i].lifeline[1] = totaltime;
				}
				if (parkingship[i].lifeline[1] + parkingship[i].outtime == totaltime)					//出泊完成
				{
					freeberth++;
					for (int j = i; j < berthqtt - freeberth; j++)
						parkingship[j] = parkingship[j + 1];
					i--;
				}
			}
			/*入泊*/
			while (freeberth && !freeship.empty ())
			{
				Cship temp = freeship.min ();
				if (10000 - t < 0.000001)
					freeship0.setbw (temp.shipno, bw0++);
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
					craninggoods[i].state = buffer;
					buffergoods.push (craninggoods[i], iw);
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
			while (freecrane)
			{
				if (!freegoods[outship].empty ())
				{
					Cgoods temp = freegoods[outship].max ();
					if (10000 - t < 0.000001)
						freeship0.setcw (temp.shipno, temp.goodsno, cw0--);
					temp.lifeline[1] = totaltime;
					temp.state = crane;
					craninggoods[craneqtt - freecrane] = temp;
					freegoods[outship].pop (pmax);
					freecrane--;
				}
				else if (outship < inship - 1)
					outship++;
				else
					break;
			}
			/*小车*/
			/*在运*/
			for (int i = 0; i < shuttleqtt - freeshuttle; i++)
			{
				if (shuttlinggoods[i].lifeline[2] + 2 * shuttletime[shuttlinggoods[i].warehouseno] == totaltime)	//运输完成
				{
					shuttlinggoods[i].state = warehouse;
					freeshuttle++;
					for (int j = i; j < shuttleqtt - freeshuttle; j++)
						shuttlinggoods[j] = shuttlinggoods[j + 1];
					i--;
				}
			}
			/*起运*/
			while (freeshuttle && !buffergoods.empty ())
			{
				Cgoods temp = buffergoods.min ();
				if (10000 - t < 0.000001)
					freeship0.setsw (temp.shipno, temp.goodsno, sw0++);
				temp.lifeline[2] = totaltime;
				temp.state = shuttle;
				shuttlinggoods[shuttleqtt - freeshuttle] = temp;
				buffergoods.pop (pmin);
				freeshuttle--;
			}
			totaltime++;
		}
		totaltime--;						
		//iw为新解，1-iw为当前解 
		if (10000 - t < 0.000001)
		{
			F0 = totaltime;
			ans = F0;
		}
		int delta = F0 - totaltime;
		if (delta >= 0)
		{
			freeship0.save (iw);
			iw = 1 - iw;				//接受新解
			F0 = totaltime;
		}
		else
		{
			double p = (double)rand () / RAND_MAX;
			if (exp (delta / t) > p)
			{
				freeship0.save (iw);
				iw = 1 - iw;
				F0 = totaltime;
			}
			else
				freeship0.save (1 - iw);
		}
		int m = 0, n = 0;
		switch(rand () % 3)					//把iw作为当前解，1-iw作为新解寻找
		{
		case 0:
			while (m == n)
			{
				m = rand () % shipqtt;    // 随机选取两艘船交换入泊优先级
				n = rand () % shipqtt;
			}
			freeship0.bswap (m, n, iw);
			break;
		case 1:
			while (m == n)
			{
				m = rand () % totalgoods;    // 随机选取两货物交换起吊优先级
				n = rand () % totalgoods;
			}
			freeship0.cswap (m, n, iw);
			break;
		case 2:
			while (m == n)
			{
				m = rand () % totalgoods;    // 随机选取两货物交换运送优先级
				n = rand () % totalgoods;
			}
			freeship0.sswap (m, n, iw);
			break;
		}
		if (F0 < ans)
			ans = F0;
		t *= 0.999;																			// 逐渐降低温度 
	}
	cout << ans;
	finish = clock ();
	runtime = (double)(finish - start);
	cout << "运行时间为" << runtime << "ms" << endl;
	return 0;
}
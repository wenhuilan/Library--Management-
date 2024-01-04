
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#define MAXM 10                     //����B�������Ľ���


int nowDate = 1;					//�����ǵ�data��
const int m = 3;                      //�趨B���Ľ���
const int Max = m - 1;                  //�������ؼ�������
const int Min = (m - 1) / 2;              //������С�ؼ�������
typedef int KeyType;                //KeyTypeΪ�ؼ�������


typedef struct {						//�����ĵ��鼮
	int tag;						//״̬�� 0->ԤԼ��1->����
	int readerId;					//����id
	int borrowTime;					//����ʱ��
	int backTime;					//�黹ʱ��
	char readerName[30];			//��������
	char bookName[30];				//�鼮����
} LendBook, * LendBookP;

typedef struct linkNode {
	int type;						//���ͣ�0->���ļ�¼��1->������¼��3->���ҵ���
	LendBook* lenBook;              //���ļ�¼
	char* log;						//������¼
	char* book;						//���ҵ���
	struct linkNode* next;
} LinkNode, * LinkType;

typedef struct {						//�鼮
	int id;							//�鼮id
	char name[30];	                //����
	double price;					//�۸�
	int authorId;					//����id
	char authorName[30];			//����
	int totalNum;					//�鼮����
	int stockNum;					//�鼮���
	LinkNode* borrowList;			//�����б�
	LinkNode* appointList;			//ԤԼ�б�
} Book;

typedef struct {						//����
	int id;							//����id
	char name[30];					//��������
	LinkNode* bookList;				//�����鼮
} Author;

typedef struct {						//����
	int id;							//����id
	char name[30];					//��������
	LinkNode* borrowList;			//�����б�
	LinkNode* appointList;			//ԤԼ�б�
} Reader;

typedef struct rcdNode {
	int type;						//�ṹ���������ͣ�
	//0->�鼮��1->���ߡ�2->����
	Book* book;						//�鼮
	Author* author;					//����
	Reader* reader;					//����
} RcdNode, * RcdType;



typedef struct BTNode {				//B����B���������
	int keynum;                     //���ؼ��ָ���
	KeyType key[MAXM];              //�ؼ������飬key[0]��ʹ��
	BTNode* parent;					//˫�׽��ָ��
	BTNode* ptr[MAXM];				//���ӽ��ָ������
	RcdNode* rcd[MAXM];
} BTNode, * BTree;

typedef struct {                     //B�����ҽ������
	BTNode* pt;                     //ָ���ҵ��Ľ��
	int i;                          //�ڽ���еĹؼ���λ��;
	int tag;                        //���ҳɹ�����־
} Result;

typedef struct LNode {               //���������������
	BTree data;                     //������
	struct LNode* next;             //ָ����
} LNode, * LinkList;

typedef enum status {               //ö�����ͣ����ε�����
	TRUE,
	FALSE,
	OK,
	ERROR,
	OVERFLOW,
	EMPTY
} Status;



//��

Status InitBTree(BTree& t) {
	//��ʼ��B��
	if (t != NULL) {
		return FALSE;
	}
	t = NULL;
	return OK;
}


int SearchBTNode(BTNode* p, KeyType k) {
	//�ڽ��p�в��ҹؼ���k�Ĳ���λ��i
	int i = 0;
	for (i = 0; i < p->keynum && p->key[i + 1] <= k; i++);
	return i;
}


Result SearchBTree(BTree t, KeyType k) {
	/*����t�ϲ��ҹؼ���k,���ؽ��(pt,i,tag)�������ҳɹ�,������ֵ
	tag=1,�ؼ���k��ָ��pt��ָ����е�i���ؼ��֣���������ֵtag=0,
	�ؼ���k�Ĳ���λ��Ϊpt���ĵ�i��*/

	BTNode* p = t, * q = NULL;                            //��ʼ�����p�ͽ��q,pָ�������,qָ��p��˫��
	int found_tag = 0;                                //�趨���ҳɹ�����־
	int i = 0;
	Result r;                                       //�趨���صĲ��ҽ��

	while (p != NULL && found_tag == 0) {
		i = SearchBTNode(p, k);                        //�ڽ��p�в��ҹؼ���k,ʹ��p->key[i]<=k<p->key[i+1]
		if (i > 0 && p->key[i] == k)                       //�ҵ�����ؼ���
			found_tag = 1;                            //���ҳɹ�
		else {                                       //����ʧ��
			q = p;
			p = p->ptr[i];
		}
	}

	if (found_tag == 1) {                               //���ҳɹ�
		r.pt = p;
		r.i = i;
		r.tag = 1;
	} else {                                         //����ʧ��
		r.pt = q;
		r.i = i;
		r.tag = 0;
	}

	return r;                                       //���عؼ���k��λ��(�����λ��)
}


void InsertBTNode(BTNode*& p, int i, KeyType k, BTNode* q, RcdNode* rcd) {
	//���ؼ���k�ͽ��q�ֱ���뵽p->key[i+1]��p->ptr[i+1]��
	int j;
	for (j = p->keynum; j > i; j--) {                       //������ƿճ�һ��λ��
		p->key[j + 1] = p->key[j];
		p->rcd[j + 1] = p->rcd[j];
		p->ptr[j + 1] = p->ptr[j];
	}
	p->key[i + 1] = k;
	p->rcd[i + 1] = rcd;
	p->ptr[i + 1] = q;
	if (q != NULL)
		q->parent = p;
	p->keynum++;
}


void SplitBTNode(BTNode*& p, BTNode*& q) {
	//�����p���ѳ��������,ǰһ�뱣��,��һ��������q
	int i;
	int s = (m + 1) / 2;
	q = (BTNode*)malloc(sizeof(BTNode));             //�����q����ռ�

	q->ptr[0] = p->ptr[s];                            //��һ��������q
	for (i = s + 1; i <= m; i++) {
		q->key[i - s] = p->key[i];
		q->rcd[i - s] = p->rcd[i];
		q->ptr[i - s] = p->ptr[i];
	}
	q->keynum = p->keynum - s;
	q->parent = p->parent;
	for (i = 0; i <= p->keynum - s; i++)                     //�޸�˫��ָ��
		if (q->ptr[i] != NULL)
			q->ptr[i]->parent = q;
	p->keynum = s - 1;                                  //���p��ǰһ�뱣��,�޸Ľ��p��keynum
}


void NewRoot(BTNode*& t, KeyType k, BTNode* p, BTNode* q, RcdNode* rcd) {
	//�����µĸ����t,ԭp��qΪ����ָ��
	t = (BTNode*)malloc(sizeof(BTNode));             //����ռ�
	t->keynum = 1;
	t->ptr[0] = p;
	t->ptr[1] = q;
	t->key[1] = k;
	t->rcd[1] = rcd;
	if (p != NULL)                                     //�������p�ͽ��q��˫��ָ��
		p->parent = t;
	if (q != NULL)
		q->parent = t;
	t->parent = NULL;
}


void InsertBTree(BTree& t, int i, KeyType k, BTNode* p, RcdNode* rcd) {
	/*����t�Ͻ��p��key[i]��key[i+1]֮�����ؼ���k��������
	������,����˫�������б�Ҫ�Ľ����ѵ���,ʹt����B��*/
	BTNode* q;
	int finish_tag, newroot_tag, s;                   //�趨��Ҫ�½���־�Ͳ�����ɱ�־
	KeyType x;
	RcdNode* xRcd;
	if (p == NULL)  //t�ǿ���
		NewRoot(t, k, NULL, NULL, rcd);  //���ɽ����ؼ���k�ĸ����t
	else {
		x = k;
		xRcd = rcd;
		q = NULL;
		finish_tag = 0;
		newroot_tag = 0;
		while (finish_tag == 0 && newroot_tag == 0) {
			InsertBTNode(p, i, x, q, xRcd);                  //���ؼ���x�ͽ��q�ֱ���뵽p->key[i+1]��p->ptr[i+1]
			if (p->keynum <= Max)
				finish_tag = 1;                       //�������
			else {
				s = (m + 1) / 2;
				SplitBTNode(p, q);                   //���ѽ��
				x = p->key[s];
				xRcd = p->rcd[s];
				if (p->parent) {                      //����x�Ĳ���λ��
					p = p->parent;
					i = SearchBTNode(p, x);
				} else                              //û�ҵ�x����Ҫ�½��
					newroot_tag = 1;
			}
		}
		if (newroot_tag == 1)                          //������ѷ���Ϊ���p��q
			NewRoot(t, x, p, q, xRcd);                       //�����¸����t,p��qΪ����ָ��
	}
}


void Remove(BTNode* p, int i) {
	//��p���ɾ��key[i]�����ĺ���ָ��ptr[i]
	int j;
	for (j = i + 1; j <= p->keynum; j++) {                    //ǰ��ɾ��key[i]��ptr[i]
		p->key[j - 1] = p->key[j];
		p->rcd[j - 1] = p->rcd[j];
		p->ptr[j - 1] = p->ptr[j];
	}
	p->keynum--;
}


void Substitution(BTNode* p, int i) {
	//���ұ�ɾ�ؼ���p->key[i](�ڷ�Ҷ�ӽ����)�����Ҷ�ӽ��(��������ֵ��С�Ĺؼ���)
	BTNode* q;
	for (q = p->ptr[i]; q->ptr[0] != NULL; q = q->ptr[0]);
	p->key[i] = q->key[1];                            //���ƹؼ���ֵ
	p->rcd[i] = q->rcd[1];
}


void MoveRight(BTNode* p, int i) {
	/*��˫�׽��p�е����һ���ؼ��������ҽ��q��
	������aq�е����һ���ؼ�������˫�׽��p��*/
	int j;
	BTNode* q = p->ptr[i];
	BTNode* aq = p->ptr[i - 1];

	for (j = q->keynum; j > 0; j--) {                       //�����ֵ�q�����йؼ�������ƶ�һλ
		q->key[j + 1] = q->key[j];
		q->rcd[j + 1] = q->rcd[j];
		q->ptr[j + 1] = q->ptr[j];
	}

	q->ptr[1] = q->ptr[0];                            //��˫�׽��p�ƶ��ؼ��ֵ����ֵ�q��
	q->key[1] = p->key[i];
	q->rcd[1] = p->rcd[i];
	q->keynum++;

	p->key[i] = aq->key[aq->keynum];                  //�����ֵ�aq�����һ���ؼ����ƶ���˫�׽��p��
	p->rcd[i] = aq->rcd[aq->keynum];
	p->ptr[i]->ptr[0] = aq->ptr[aq->keynum];
	aq->keynum--;
}


void MoveLeft(BTNode* p, int i) {
	/*��˫�׽��p�еĵ�һ���ؼ�����������aq�У�
	���ҽ��q�еĵ�һ���ؼ�������˫�׽��p��*/
	int j;
	BTNode* aq = p->ptr[i - 1];
	BTNode* q = p->ptr[i];

	aq->keynum++;                                   //��˫�׽��p�еĹؼ����ƶ������ֵ�aq��
	aq->key[aq->keynum] = p->key[i];
	aq->rcd[aq->keynum] = p->rcd[i];
	aq->ptr[aq->keynum] = p->ptr[i]->ptr[0];

	p->key[i] = q->key[1];                            //�����ֵ�q�еĹؼ����ƶ���˫�׽ڵ�p��
	p->rcd[i] = q->rcd[1];
	q->ptr[0] = q->ptr[1];
	q->keynum--;

	for (j = 1; j <= q->keynum; j++) {                     //�����ֵ�q�����йؼ�����ǰ�ƶ�һλ
		q->key[j] = q->key[j + 1];
		q->rcd[j] = q->rcd[j + 1];
		q->ptr[j] = q->ptr[j + 1];
	}
}


void Combine(BTNode* p, int i) {
	/*��˫�׽��p���ҽ��q�ϲ�������aq��
	������˫�׽��p�е�ʣ��ؼ��ֵ�λ��*/
	int j;
	BTNode* q = p->ptr[i];
	BTNode* aq = p->ptr[i - 1];

	aq->keynum++;                                  //��˫�׽��Ĺؼ���p->key[i]���뵽����aq
	aq->key[aq->keynum] = p->key[i];
	aq->rcd[aq->keynum] = p->rcd[i];
	aq->ptr[aq->keynum] = q->ptr[0];

	for (j = 1; j <= q->keynum; j++) {                      //���ҽ��q�е����йؼ��ֲ��뵽����aq
		aq->keynum++;
		aq->key[aq->keynum] = q->key[j];
		aq->rcd[aq->keynum] = q->rcd[j];
		aq->ptr[aq->keynum] = q->ptr[j];
	}

	for (j = i; j < p->keynum; j++) {                       //��˫�׽��p�е�p->key[i]������йؼ�����ǰ�ƶ�һλ
		p->key[j] = p->key[j + 1];
		p->rcd[j] = p->rcd[j + 1];
		p->ptr[j] = p->ptr[j + 1];
	}
	p->keynum--;                                    //�޸�˫�׽��p��keynumֵ
	free(q);                                        //�ͷſ��ҽ��q�Ŀռ�
}


void AdjustBTree(BTNode* p, int i) {
	//ɾ�����p�еĵ�i���ؼ��ֺ�,����B��
	if (i == 0)                                        //ɾ����������߹ؼ���
		if (p->ptr[1]->keynum > Min)                   //�ҽ����Խ�
			MoveLeft(p, 1);
		else                                        //���ֵܲ�����
			Combine(p, 1);
	else if (i == p->keynum)                           //ɾ���������ұ߹ؼ���
		if (p->ptr[i - 1]->keynum > Min)                 //������Խ�
			MoveRight(p, i);
		else                                        //���㲻����
			Combine(p, i);
	else if (p->ptr[i - 1]->keynum > Min)                //ɾ���ؼ������в������㹻��
		MoveRight(p, i);
	else if (p->ptr[i + 1]->keynum > Min)                //ɾ���ؼ������в����ҽ�㹻��
		MoveLeft(p, i + 1);
	else                                            //ɾ���ؼ������в������ҽ�㶼������
		Combine(p, i);
}


int FindBTNode(BTNode* p, KeyType k, int& i) {
	//��ӳ�Ƿ��ڽ��p���Ƿ���ҵ��ؼ���k
	if (k < p->key[1]) {                                //���p�в��ҹؼ���kʧ��
		i = 0;
		return 0;
	} else {                                         //��p����в���
		i = p->keynum;
		while (k < p->key[i] && i > 1)
			i--;
		if (k == p->key[i])                            //���p�в��ҹؼ���k�ɹ�
			return 1;
	}
	return 0;
}


int BTNodeDelete(BTNode* p, KeyType k) {
	//�ڽ��p�в��Ҳ�ɾ���ؼ���k
	int i;
	int found_tag;                                  //���ұ�־
	if (p == NULL)
		return 0;
	else {
		found_tag = FindBTNode(p, k, i);                //���ز��ҽ��
		if (found_tag == 1) {                           //���ҳɹ�
			if (p->ptr[i - 1] != NULL) {                  //ɾ�����Ƿ�Ҷ�ӽ��
				Substitution(p, i);                  //Ѱ�����ڹؼ���(����������С�Ĺؼ���)
				BTNodeDelete(p->ptr[i], p->key[i]);  //ִ��ɾ������
			} else
				Remove(p, i);                        //�ӽ��p��λ��i��ɾ���ؼ���
		} else
			found_tag = BTNodeDelete(p->ptr[i], k);    //�غ��ӽ��ݹ���Ҳ�ɾ���ؼ���k
		if (p->ptr[i] != NULL)
			if (p->ptr[i]->keynum < Min)               //ɾ����ؼ��ָ���С��MIN
				AdjustBTree(p, i);                   //����B��
		return found_tag;
	}
}


void BTreeDelete(BTree& t, KeyType k) {
	//����ɾ����ܣ�ִ��ɾ������
	BTNode* p;
	int a = BTNodeDelete(t, k);                        //ɾ���ؼ���k
	if (a == 0) {}                                       //����ʧ��
	else if (t->keynum == 0) {                          //����
		p = t;
		t = t->ptr[0];
		free(p);
	}
}


void DestroyBTree(BTree& t) {
	//�ݹ��ͷ�B��
	int i;
	BTNode* p = t;
	if (p != NULL) {                                    //B����Ϊ��
		for (i = 0; i <= p->keynum; i++) {                  //�ݹ��ͷ�ÿһ�����
			DestroyBTree(*&p->ptr[i]);
		}
		free(p);
	}
	t = NULL;
}

Status InitQueue(LinkList& L) {
	//��ʼ������
	L = (LNode*)malloc(sizeof(LNode));                //������ռ�
	if (L == NULL)                                     //����ʧ��
		return OVERFLOW;
	L->next = NULL;
	return OK;
}


LNode* CreateNode(BTNode* p) {
	//�½�һ�����
	LNode* q;
	q = (LNode*)malloc(sizeof(LNode));                //������ռ�
	if (q != NULL) {                                    //����ɹ�
		q->data = p;
		q->next = NULL;
	}
	return q;
}


Status Enqueue(LNode* p, BTNode* q) {
	//Ԫ��q�����
	if (p == NULL)
		return ERROR;
	while (p->next != NULL)                            //�����������
		p = p->next;
	p->next = CreateNode(q);                          //���ɽ����q�������
	return OK;
}


Status Dequeue(LNode* p, BTNode*& q) {
	//�����У�����q����ֵ
	LNode* aq;
	if (p == NULL || p->next == NULL)                      //ɾ��λ�ò�����
		return ERROR;
	aq = p->next;                                     //�޸ı�ɾ���aq��ָ����
	p->next = aq->next;
	q = aq->data;
	free(aq);                                       //�ͷŽ��aq
	return OK;
}


Status IfEmpty(LinkList L) {
	//�����п�
	if (L == NULL)                                     //���в�����
		return ERROR;
	if (L->next == NULL)                               //����Ϊ��
		return TRUE;
	return FALSE;                                   //���зǿ�
}

void DestroyQueue(LinkList L) {
	//���ٶ���
	LinkList p;
	if (L != NULL) {
		p = L;
		L = L->next;
		free(p);                                    //��һ�ͷ�
		DestroyQueue(L);
	}
}

Status Traverse(BTree t, LinkList L, int newline, int sum) {
	//�ö��б������B��
	int i;
	BTree p;
	if (t != NULL) {
		printf("[ ");
		Enqueue(L, t->ptr[0]);                       //���
		for (i = 1; i <= t->keynum; i++) {
			printf(" %d��", t->key[i]);
			if (t->rcd[i]->type == 0) {
				printf("��%s��", t->rcd[i]->book->name);
			} else if (t->rcd[i]->type == 1) {
				printf("%s", t->rcd[i]->author->name);
			} else if (t->rcd[i]->type == 2) {
				printf("%s", t->rcd[i]->reader->name);
			}
			Enqueue(L, t->ptr[i]);                   //�ӽ�����
		}
		sum += t->keynum + 1;
		printf(" ]");
		if (newline == 0) {                             //��Ҫ����һ��
			printf("\n");
			newline = sum - 1;
			sum = 0;
		} else
			newline--;
	}

	if (IfEmpty(L) == FALSE) {                         //l��Ϊ��
		Dequeue(L, p);                              //���ӣ���p����
		Traverse(p, L, newline, sum);                 //�������ӽ��
	}
	return OK;
}


Status PrintBTree(BTree t) {
	//���B��
	LinkList L;
	if (t == NULL) {
		printf("  B��Ϊ����");
		return OK;
	}
	InitQueue(L);                                   //��ʼ������
	Traverse(t, L, 0, 0);                              //���ö������
	DestroyQueue(L);                                //���ٶ���
	return OK;
}

// ��ʼ������
void initLend(LinkType& list) {
	list = (LinkNode*)malloc(sizeof(LinkNode));
	list->next = NULL;
}
// ��ӽڵ�,���ؽڵ�ָ��
LinkType addLinkNode(LinkType& list, int type) {
	LinkNode* p;
	p = list;
	while (p->next != NULL) {
		p = p->next;
	}
	p->next = (LinkNode*)malloc(sizeof(LinkNode));
	p->next->next = NULL;
	p->type = type;
	return p->next;
}

//����


// ��������
int inputInt() {
	int a, result;
	fflush(stdin);
	a = scanf("%d", &result);
	if (a == 0) {
		printf("\n���������֡������������������: ");
		result = inputInt();
	}
	return result;
}


// ��ʼ��
// �������
RcdNode* addAuthor(BTree& t, int id, const char* name) {

	Result s2 = SearchBTree(t, id);
	RcdNode* rcd = (RcdNode*)malloc(sizeof(RcdNode));
	if (rcd == NULL) {
		return NULL;
	}
	rcd->author = (Author*)malloc(sizeof(Author));
	if (rcd->author == NULL) {
		return NULL;
	}
	rcd->type = 1;
	rcd->author->id = id;
	strcpy(rcd->author->name, name);

	initLend(rcd->author->bookList);
	InsertBTree(t, s2.i, id, s2.pt, rcd);
	return rcd;
}
// ����������鼮
void addAuthorBook(Author*& author, const char* bookName) {
	//LinkType addLinkNode(LinkType & list, int type);
	LinkNode* p = addLinkNode(author->bookList, 3);
	p->book = (char*)malloc(sizeof(char) * 20);
	if (p->book == NULL) {
		return;
	}
	strcpy(p->book, bookName);
}
// ������ɾ���鼮
int deleteAuthorBook(Author*& author, char* bookName) {
	LinkNode* p, * pre;
	p = author->bookList;
	while (p->next != NULL && strcmp(p->next->book, bookName) != 0) {
		p = p->next;
	}
	if (p->next == NULL) {
		return 0; // �Ҳ��������¼
	}
	pre = p;
	p = p->next;
	pre->next = p->next;
	free(p);
	return 1;
}
void writeBookInfoToFile(const std::string& filename, const char* name, int id, double price) {
std::ofstream file(filename, std::ios::binary | std::ios::app);
if (file.is_open()) {
file.seekp(0, std::ios::end);
file.write(reinterpret_cast<const char*>(&id), sizeof(id));
file.write(name, strlen(name) + 1); //д���ַ���ʱ��Ҫ���Ͻ�β��'\0'
file.write(reinterpret_cast<const char*>(&price), sizeof(price));
file.close();
} else {
std::cout << "�޷����ļ�" << std::endl;
}
}

void readBookInfoFromFile(const std::string& filename) {
std::ifstream file(filename, std::ios::binary);
if (file.is_open()) {
file.seekg(0, std::ios::end);
std::streampos length = file.tellg(); //��ȡ�ļ�����
file.seekg(0, std::ios::beg);
while (file.tellg() < length) {
int id;
char buffer[256]; //���������Ϊ255���ַ�
double price;
file.read(reinterpret_cast<char*>(&id), sizeof(id));
file.read(buffer, sizeof(buffer));
file.read(reinterpret_cast<char*>(&price), sizeof(price));
std::string name(buffer); //���ַ�����ת���ַ���
std::cout << "��ţ�" << id << ", ������" << name << ", �۸�" << price << std::endl;
}
file.close();
} else {
std::cout << "�޷����ļ�" << std::endl;
}
}
// ����鼮
int addBookFun(BTree& t, int id, const char* name, BTree& author, int authorId, double price) {

	Result s = SearchBTree(t, id);
	if (s.tag == 1) {
		Book* book = s.pt->rcd[s.i]->book;
		book->totalNum++;
		book->stockNum++;
	} else {
		RcdNode* rcd = (RcdNode*)malloc(sizeof(RcdNode));
		if (rcd == NULL) {
			return 0;
		}
		rcd->book = (Book*)malloc(sizeof(Book));
		if (rcd->book == NULL) {
			return 0;
		}
		rcd->type = 0;
		rcd->book->id = id;
		strcpy(rcd->book->name, name);
		writeBookInfoToFile("books.txt", name, id, price); //д���鼮��Ϣ���ļ�
		Result s2 = SearchBTree(author, authorId);
		if (s2.tag == 0) {
			RcdNode* rcd2 = addAuthor(author, authorId, "��֪������");
			addAuthorBook(rcd2->author, name);
			strcpy(rcd->book->authorName, "��֪������");
		} else {
			char* authorName = s2.pt->rcd[s2.i]->author->name;
			Author* p2 = s2.pt->rcd[s2.i]->author;
			addAuthorBook(p2, name);
			strcpy(rcd->book->authorName, authorName);
		}
		//
		rcd->book->totalNum = 1;
		rcd->book->stockNum = 1;
		rcd->book->authorId = authorId;
		initLend(rcd->book->borrowList);
		initLend(rcd->book->appointList);
		InsertBTree(t, s.i, id, s.pt, rcd);
	}
	return 1;
}
// ��Ӷ���
void addReader(BTree& t, int id, const char* name) {
	//Result SearchBTree(BTree t, KeyType k);
	//void initLend(LinkType & list);
	//void InsertBTree(BTree & t, int i, KeyType k, BTNode * p, RcdNode * rcd);
	Result s = SearchBTree(t, id);
	if (s.tag == 1) {
		Reader* reader = s.pt->rcd[s.i]->reader;

	} else {
		RcdNode* rcd = (RcdNode*)malloc(sizeof(RcdNode));
		if (rcd == NULL) {
			return;
		}
		rcd->reader = (Reader*)malloc(sizeof(Reader));
		if (rcd->reader == NULL) {
			return;
		}
		rcd->type = 2;
		rcd->reader->id = id;
		strcpy(rcd->reader->name, name);
		initLend(rcd->reader->borrowList);
		initLend(rcd->reader->appointList);
		InsertBTree(t, s.i, id, s.pt, rcd);
	}
}
void init(BTree& book, BTree& author, BTree& reader, LinkNode*& Log) {
	//void initLend(LinkType & list);
	//��ʼ������
	addAuthor(author, 1, "��ѩ��");
	addAuthor(author, 2, "���°Գ�");
	addAuthor(author, 3, "��������");
	addAuthor(author, 4, "����");
	addAuthor(author, 5, "·ң");
	addAuthor(author, 6, "��ͬ");
	addAuthor(author, 7, "����");
	//��ʼ���鼮
	addBookFun(book, 1, "��¥��", author, 1, 36.5);
	addBookFun(book, 1, "��¥��", author, 1, 36.5);
	addBookFun(book, 1, "��¥��", author, 1, 36.5);
	addBookFun(book, 1, "��¥��", author, 1, 36.5);
	addBookFun(book, 1, "��¥��", author, 1, 36.5);
	addBookFun(book, 2, "����", author, 2, 30.8);
	addBookFun(book, 3, "��Ĺ�ʼ�", author, 3, 40.5);
	addBookFun(book, 4, "���¾�", author, 4, 42.6);
	addBookFun(book, 5, "ƽ��������", author, 5, 55.5);
	addBookFun(book, 6, "��Ĺ¶����������", author, 6, 60.7);
	addBookFun(book, 7, "����IV", author, 7, 120.5);

	//��ʼ������
	addReader(reader, 1001, "СA");
	addReader(reader, 1002, "СB");
	addReader(reader, 1003, "СC");
	//��ʼ����־
	initLend(Log);
}

// ������־
void addLog(LinkType& list, char* str) {
	//	LinkType addLinkNode(LinkType & list, int type);
	LinkNode* p = addLinkNode(list, 1);
	p->log = (char*)malloc(sizeof(char) * 50);
	if (p->log == NULL) {
		return;
	}
	strcpy(p->log, str);
}

void log(LinkNode*& Log) {
	int i = 1;
	LinkNode* p;
	p = Log->next;
	if (p == NULL) {
		printf("\n��������־������\n");
		return;
	}
	printf("\n��������־��:\n ");
	while (p != NULL) {
		printf("\n            %d��", i);
		printf("  %s\n", p->log);
		i++;
		p = p->next;
	}
	printf("\n");
}

// ͼ�����
void addBook(BTree& t, BTree& author, LinkNode*& Log) {
	//Result SearchBTree(BTree t, KeyType k);
	int id, authorId;
	double price;
	char name[30], authorName[30], str[50];
	printf("\n��ͼ����⡿������[���]��");
	id = inputInt();
	Result s = SearchBTree(t, id);
	if (s.tag == 1) {
		Book* book = s.pt->rcd[s.i]->book;
		printf("\n��ͼ����⡿�鼮��%s������%s(���: %d) �Ѿ����ڣ��Ƿ����ӿ��������ȷ��������1������������������ ", book->name, book->authorName, book->id);
		if (inputInt() == 1) {
			book->totalNum++;
			book->stockNum++;
			printf("\n��ͼ����⡿��ӳɹ����鼮��%s������Ϊ %d\n", book->name, book->totalNum);
			sprintf(str, "��[%d]�죬ͼ�顶%s���������+1", nowDate, book->name);
		} else {
			printf("\n��ͼ����⡿ȡ���ɹ�\n");
			sprintf(str, "��[%d]�죬ȡ��ͼ�顶%s�������������", nowDate, book->name);
		}
	} else {
		printf("\n��ͼ����⡿������[����]��");
		fflush(stdin);
		fgets(name, sizeof(name), stdin);
		printf("\n��ͼ����⡿������[�۸�]��");
		price = inputInt();
		printf("\n��ͼ����⡿������[�������]��");
		authorId = inputInt();
		Result s2 = SearchBTree(author, authorId);
		if (s2.tag == 0) {
			printf("\n��ͼ����⡿������[��������]��");
			fflush(stdin);
			scanf("%s", authorName);
			addAuthor(author, authorId, authorName);
		}
		if (1 == addBookFun(t, id, name, author, authorId, price)) {
			printf("\n��ͼ����⡿����鼮��%s���ɹ�\n", name);
			sprintf(str, "��[%d]�죬����鼮��%s���ɹ�", nowDate, name);
		} else {
			printf("\n��ͼ����⡿�ڴ�ռ����ʧ��\n");
		}
	}
	addLog(Log, str);
}
//���ļ�����ɾ����Ӧ��ŵ�ͼ����Ϣ
void deleteBookFromFile(const std::string& filename, int idToDelete) {
	std::ifstream inFile(filename);
	if (!inFile.is_open()) {
		std::cout << "�޷����ļ�" << std::endl;
		return;
	}

	std::ofstream outFile("temp.txt"); // ����һ����ʱ�ļ�

	if (!outFile.is_open()) {
		std::cout << "�޷�������ʱ�ļ�" << std::endl;
		inFile.close();
		return;
	}

	int id;
	std::string name;
	double price;

	while (inFile >> id >> name >> price) {
		if (id != idToDelete) {
			outFile << id << " " << name << " " << price << std::endl;
		}
	}

	inFile.close();
	outFile.close();

	if (std::remove(filename.c_str()) != 0) {
		std::cout << "�޷�ɾ��ԭ�ļ�" << std::endl;
		return;
	}

	if (std::rename("temp.txt", filename.c_str()) != 0) {
		std::cout << "�޷���������ʱ�ļ�" << std::endl;
		return;
	}
}
void clearFile(const std::string& filename) {
	std::ofstream file(filename, std::ios::trunc);
	if (file.is_open()) {
		file.close();
		std::cout << "�ļ��ѳɹ����" << std::endl;
	} else {
		std::cout << "�޷����ļ�" << std::endl;
	}
}

// ɾ��ͼ��
void deleteBook(BTree& t, BTree& author, LinkNode*& Log) {
	//Result SearchBTree(BTree t, KeyType k);
	//void BTreeDelete(BTree & t, KeyType k);
	int id;
	printf("\n��ɾ��ͼ�顿������[���]��");
	id = inputInt();
	Result s = SearchBTree(t, id);
	if (s.tag == 0) {
		printf("\n��ɾ��ͼ�顿���鲻����\n");
		char str[50];
		sprintf(str, "��[%d]�죬ɾ��ͼ��[%d]ʧ�ܣ����鲻����", nowDate, id);
		addLog(Log, str);
	} else {
		Book* book = s.pt->rcd[s.i]->book;
		printf("\n��ɾ��ͼ�顿�Ƿ�ȷ��ɾ���鼮��%s������%s(���: %d)��ȷ��������1������������������ ", book->name, book->authorName, book->id);
		if (inputInt() == 1) {
			if (book->borrowList != NULL) {
				free(book->borrowList);
			}
			if (book->appointList != NULL) {
				free(book->appointList);
			}
			Result s2 = SearchBTree(author, book->authorId);
			if (0 == deleteAuthorBook(s2.pt->rcd[s2.i]->author, book->name)) {
				printf("\n\n�Ҳ��������¼\n\n");
			}
			char str[50];
			deleteBookFromFile("books.txt", book->id);
			sprintf(str, "��[%d]�죬ɾ��ͼ�顶%s���ɹ�", nowDate, book->name);
			addLog(Log, str);
			free(book);
			free(s.pt->rcd[s.i]);
			BTreeDelete(t, id);
			printf("\n��ɾ��ͼ�顿ɾ���ɹ�\n");
		} else {
			printf("\n��ɾ��ͼ�顿ȡ���ɹ�\n");
			char str[50];
			sprintf(str, "��[%d]�죬ȡ��ɾ��ͼ�顶%s��", nowDate, book->name);
			addLog(Log, str);
		}
	}

}

// ��ӽ��ļ�¼�ڵ�
void addLendBook(LinkType& list, int readerId, char* readerName, char* bookName, int tag, int borrowTime, int backTime) {
	//LinkType addLinkNode(LinkType & list, int type);
	LinkNode* p = addLinkNode(list, 0);
	if (p == NULL) {
		return;
	}
	p->lenBook = (LendBook*)malloc(sizeof(LendBook));
	if (p->lenBook == NULL) {
		return;
	}
	p->lenBook->tag = tag;
	p->lenBook->readerId = readerId;
	p->lenBook->borrowTime = borrowTime;
	p->lenBook->backTime = backTime;
	strcpy(p->lenBook->readerName, readerName);
	strcpy(p->lenBook->bookName, bookName);
}


// ƥ���鼮
Result searchBook(BTree t, const char* str) {
	//Result SearchBTree(BTree t, KeyType k);
	int id;
	id = inputInt();
	Result s = SearchBTree(t, id);
	if (s.tag == 0) {
		printf("\n��%s�����鲻����, ���������룺", str);
		s = searchBook(t, str);
	}
	return s;
}
// ƥ�����
Result searchReader(BTree t, const char* str) {
	//Result SearchBTree(BTree t, KeyType k);
	int id;
	id = inputInt();
	Result s = SearchBTree(t, id);
	if (s.tag == 0) {
		printf("\n��%s���ý���֤�Ų�����, ���������룺", str);
		s = searchReader(t, str);
	}
	return s;
}
// �������ļ�¼
LinkType findLinkNode(LinkType& list, char* name, int type) {
	LinkNode* p;
	p = list;
	// type: �������ͣ�
	// 0->������Ķ���¼��������
	// 1->������Ķ���¼�����鼮
	if (type == 0) {
		while (p->next != NULL && strcmp(p->next->lenBook->readerName, name) != 0) {
			p = p->next;
		}
	} else {
		while (p->next != NULL && strcmp(p->next->lenBook->bookName, name) != 0) {
			p = p->next;
		}
	}
	if (p->next == NULL) {
		return NULL; // �Ҳ��������ؿ�
	} else {
		return p; // �ҵ���������һ���ڵ�ָ��
	}
}
// ɾ�����ļ�¼�ڵ�
int deleteLendBook(LinkType& list, char* name, int type) {
	LinkNode* p, * pre;
	p = findLinkNode(list, name, type);
	if (p == NULL) {
		return 0;
	}
	free(p->next->lenBook);
	pre = p;
	p = p->next;
	pre->next = p->next;
	free(p);
	return 1;
}
// ����֮��ɹ�ԤԼ
int appointSuccess(Book*& book, BTree& t) {
	//	Result SearchBTree(BTree t, KeyType k);
	LinkNode* appoint = book->appointList;
	if (appoint->next == NULL) {
		return 0;
	}
	LinkNode* p = appoint->next;
	LendBook* lend = p->lenBook;
	int readerId = lend->readerId;
	Result s = SearchBTree(t, readerId);
	Reader* reader = s.pt->rcd[s.i]->reader;
	printf("\n��ԤԼ���顿[%s]�����С�%s����ԤԼ��¼����Ϊ���ɹ�ԤԼ\n", lend->readerName, lend->bookName);
	addLendBook(book->borrowList, readerId, lend->readerName, lend->bookName, 1, nowDate, nowDate + lend->backTime);
	deleteLendBook(reader->appointList, book->name, 1);
	addLendBook(reader->borrowList, readerId, lend->readerName, lend->bookName, 1, nowDate, nowDate + lend->backTime);
	free(p->lenBook);
	appoint->next = p->next;
	free(p);
	return 1;
}
// ��������
int dataNumInput() {
	int res = inputInt();
	if (res <= 0) {
		printf("\n�����顿���������������0�죬���������룺");
		res = dataNumInput();
	}
	return res;
}
// ԤԼ
void appointBook(BTree& t, BTree& reader, LinkNode*& Log) {

	printf("\n��ԤԼ���顿������[���]��");
	Result s = searchBook(t, "ԤԼ����");
	Book* book = s.pt->rcd[s.i]->book;
	if (book->stockNum > 0) {
		printf("\n��ԤԼ���顿����ԤԼ���������п�棬����ֱ�ӽ���\n");
		char str[50];
		sprintf(str, "��[%d]�죬ԤԼ���顶%s��ʧ��, �������п��", nowDate, book->name);
		addLog(Log, str);
	} else {
		printf("\n��ԤԼ���顿������[����֤��](��ʾ��1001 / 1002/ 1003)��");
		Result s2 = searchReader(reader, "ԤԼ����");
		Reader* p = s2.pt->rcd[s2.i]->reader;
		if (findLinkNode(p->appointList, book->name, 1) != NULL) {
			printf("\n��ԤԼ���顿��Ǹ[%s]�������ظ�ԤԼ���顶%s��\n", p->name, book->name);
			char str[50];
			sprintf(str, "��[%d]�죬[%s]ԤԼ���顶%s��ʧ�ܣ��ظ�ԤԼ����", nowDate, p->name, book->name);
			addLog(Log, str);
			return;
		}
		printf("\n��ԤԼ���顿��ã�[%s]��������ҪԤԼ���顶%s��(���: %d)�����죺 ", p->name, book->name, book->id);
		int dataNum = dataNumInput();
		addLendBook(book->appointList, p->id, p->name, book->name, 0, 0, dataNum);
		addLendBook(p->appointList, p->id, p->name, book->name, 0, 0, dataNum);
		printf("\n��ԤԼ���顿ԤԼ�ɹ�\n");
		char str[50];
		sprintf(str, "��[%d]�죬[%s]ԤԼ���顶%s���ɹ�", nowDate, p->name, book->name);
		addLog(Log, str);
	}
}
// ����
void borrowBook(BTree& t, BTree& reader, LinkNode*& Log) {

	printf("\n�����顿������[���]��");
	Result s = searchBook(t, "����");
	Book* book = s.pt->rcd[s.i]->book;
	if (book->stockNum <= 0) {
		printf("\n�����顿��Ǹ�����Ѿ�û�п�棬��Ҫ���鷳��ԤԼ\n");
		char str[50];
		sprintf(str, "��[%d]�죬���顶%s��ʧ��, û�п��", nowDate, book->name);
		addLog(Log, str);
	} else {
		printf("\n�����顿������[����֤��](��ʾ��1001 / 1002 / 1003)��");
		Result s2 = searchReader(reader, "����");
		Reader* p = s2.pt->rcd[s2.i]->reader;
		if (findLinkNode(p->borrowList, book->name, 1) != NULL) {
			printf("\n�����顿��Ǹ[%s]�������ظ����顶%s��\n", p->name, book->name);
			char str[50];
			sprintf(str, "��[%d]�죬[%s]���顶%s��ʧ�ܣ��ظ�����", nowDate, p->name, book->name);
			addLog(Log, str);
			return;
		}
		printf("\n�����顿��ã�[%s]��������Ҫ���顶%s��(���: %d)�����죺 ", p->name, book->name, book->id);
		int dataNum = dataNumInput();
		addLendBook(book->borrowList, p->id, p->name, book->name, 1, nowDate, nowDate + dataNum);
		addLendBook(p->borrowList, p->id, p->name, book->name, 1, nowDate, nowDate + dataNum);
		book->stockNum--;
		printf("\n�����顿����ɹ�\n");
		char str[50];
		sprintf(str, "��[%d]�죬[%s]���顶%s���ɹ�", nowDate, p->name, book->name);
		addLog(Log, str);
	}
}
// ����
void backBook(BTree& t, BTree& reader, LinkNode*& Log) {
	printf("\n�����顿������[���]��");
	Result s = searchBook(t, "����");
	Book* book = s.pt->rcd[s.i]->book;
	if (book->stockNum == book->totalNum) {
		printf("\n�����顿��Ǹ������û�н��ļ�¼\n");
		char str[50];
		sprintf(str, "��[%d]�죬���顶%s��ʧ�ܣ�����û�н��ļ�¼", nowDate, book->name);
		addLog(Log, str);
	} else {
		printf("\n�����顿������[����֤��](��ʾ��1001 / 1002 / 1003)��");
		Result s2 = searchReader(reader, "����");
		Reader* p = s2.pt->rcd[s2.i]->reader;
		if (deleteLendBook(p->borrowList, book->name, 1) == 1) {
			deleteLendBook(book->borrowList, p->name, 0);
			book->stockNum++;
			printf("\n�����顿��ã�[%s],���顶%s���ɹ�\n", p->name, book->name);
			if (1 == appointSuccess(book, reader)) {
				book->stockNum--;
			}
			char str[50];
			sprintf(str, "��[%d]�죬[%s]���顶%s���ɹ�", nowDate, p->name, book->name);
			addLog(Log, str);
		} else {
			printf("\n�����顿����ʧ��[%s]û�н��Ĺ���%s��\n", p->name, book->name);
			char str[50];
			sprintf(str, "��[%d]�죬����ʧ�ܣ�[%s]û�н��Ĺ���%s��", nowDate, p->name, book->name);
			addLog(Log, str);
		}
	}

}

// ��ѯͼ��
// ��ӡ���ļ�¼
void printLendBook(LinkType& list, int type) {
	int i = 1;
	LinkNode* p;
	p = list->next;
	if (p == NULL) {
		printf("��\n");
		return;
	}
	printf("\n");
	while (p != NULL) {
		LendBook* lend = p->lenBook;
		printf("\n            %d��", i);
		if (lend->backTime < nowDate && lend->tag == 1) {
			printf("�������ڡ�");
		}
		if (type == 0) {
			if (lend->tag == 1) {
				printf("[%s (%d)]�ڵ�%d�ս��ߣ����ڵ�%d�չ黹\n", lend->readerName, lend->readerId, lend->borrowTime, lend->backTime);
			} else if (p->lenBook->tag == 0) {
				printf("[%s (%d)]ԤԼ����%d��\n", lend->readerName, lend->readerId, lend->backTime - lend->borrowTime);
			}
		} else {
			if (lend->tag == 1) {
				printf("��%s���ڵ�%d�ս��ߣ����ڵ�%d�չ黹\n", lend->bookName, lend->borrowTime, lend->backTime);
			} else if (p->lenBook->tag == 0) {
				printf("��%s��ԤԼ����%d��\n", lend->bookName, lend->backTime - lend->borrowTime);
			}
		}
		i++;
		p = p->next;
	}
}
// ���ܺ���
void bookData(BTree& t, LinkNode*& Log) {
	//Result SearchBTree(BTree t, KeyType k);
	int id;
	printf("\n����ѯͼ�顿������[���]��");
	id = inputInt();
	Result s = SearchBTree(t, id);
	if (s.tag == 0) {
		printf("\n����ѯͼ�顿���鲻����\n");
		char str[50];
		sprintf(str, "��[%d]�죬��ѯͼ��[%d]ʧ�ܣ����鲻����", nowDate, id);
		addLog(Log, str);
	} else {
		Book* book = s.pt->rcd[s.i]->book;
		printf("\n����ѯͼ�顿���ҳɹ�: \n");
		printf("\n	��ţ�%d\n", book->id);
		printf("\n	��������%s��\n", book->name);
		printf("\n	���ߣ�%s\n", book->authorName);
		printf("\n	������%d\n", book->totalNum);
		printf("\n	���������%d\n", book->stockNum);
		printf("\n	���������");
		printLendBook(book->borrowList, 0);
		printf("\n	ԤԼ�����");
		printLendBook(book->appointList, 0);
		char str[50];
		sprintf(str, "��[%d]�죬��ѯͼ��[%d]�ɹ�������%s��", nowDate, id, book->name);
		addLog(Log, str);
	}
}

// ��ѯ����
// ��ӡ����ȫ���鼮
void printAuthorBook(LinkType& list) {
	int i = 1;
	LinkNode* p;
	p = list->next;
	if (p == NULL) {
		printf("��\n");
		return;
	}
	while (p != NULL) {
		printf("��%s�� ", p->book);
		i++;
		p = p->next;
	}
	printf("\n");
}
// ���ܺ���
void authorData(BTree& t, LinkNode*& Log) {
	//Result SearchBTree(BTree t, KeyType k);
	int id;
	printf("\n����ѯ���ߡ�������[�������]��");
	id = inputInt();
	Result s = SearchBTree(t, id);
	if (s.tag == 0) {
		printf("\n����ѯ���ߡ������߲�����\n");
		char str[50];
		sprintf(str, "��[%d]�죬��ѯ����[%d]ʧ��, �����߲�����", nowDate, id);
		addLog(Log, str);
	} else {
		Author* author = s.pt->rcd[s.i]->author;
		printf("\n����ѯ���ߡ����ҳɹ�: \n");
		printf("\n	��ţ�%d\n", author->id);
		printf("\n	���ߣ�%s\n", author->name);
		printf("\n	������");
		printAuthorBook(author->bookList);
		char str[50];
		sprintf(str, "��[%d]�죬��ѯ����[%d]�ɹ�����%s", nowDate, id, author->name);
		addLog(Log, str);
	}
}

// ��ѯ����
void readerData(BTree& t, LinkNode*& Log) {
	//void printLendBook(LinkType & list, int type);
	//Result SearchBTree(BTree t, KeyType k);
	int id;
	printf("\n����ѯ���ߡ�������[����֤��]��");
	id = inputInt();
	Result s = SearchBTree(t, id);
	if (s.tag == 0) {
		printf("\n����ѯ���ߡ��ö��߲�����\n");
		char str[50];
		sprintf(str, "��[%d]�죬��ѯ����[%d]ʧ��, �ö��߲�����", nowDate, id);
		addLog(Log, str);
	} else {
		Reader* reader = s.pt->rcd[s.i]->reader;
		printf("\n����ѯ���ߡ����ҳɹ�: \n");
		printf("\n	����֤�ţ�%d\n", reader->id);
		printf("\n	���֣�%s\n", reader->name);
		printf("\n	���������");
		printLendBook(reader->borrowList, 1);
		printf("\n	ԤԼ�����");
		printLendBook(reader->appointList, 1);
		char str[50];
		sprintf(str, "��[%d]�죬��ѯ����[%d]�ɹ�����%s", nowDate, id, reader->name);
		addLog(Log, str);
	}
}

// �԰�������ʽ��ʾB��
void AoTuPrint(BTree t, int depth) {

	int i, j;
	if (t == NULL)
		return;
	printf("\t");
	for (j = 0; j < depth; j++) {
		printf("\t\t");
	}
	for (i = 1; i <= t->keynum; i++) {
		printf("[ ");
		printf("%d: ", t->key[i]);
		if (t->rcd[i]->type == 0) {
			printf("��%s��", t->rcd[i]->book->name);
		} else if (t->rcd[i]->type == 1) {
			printf("%s", t->rcd[i]->author->name);
		} else if (t->rcd[i]->type == 2) {
			printf("%s", t->rcd[i]->reader->name);
		}
		printf(" ]");
		if (i != t->keynum) {
			printf(",");
		}
	}
	putchar('\n');
	for (i = 0; i <= t->keynum; i++) {
		AoTuPrint(t->ptr[i], depth + 1);
	}
}

// �鿴ȫ��ͼ�飨��ӡB����
void printBook(BTree& book, BTree& author, BTree& reader, LinkNode*& Log) {
	printf("\n��ȫ��ͼ�顿\n");
	AoTuPrint(book, 0);
	//PrintBTree(book);
	printf("\n��ȫ�����ߡ�\n");
	AoTuPrint(author, 0);
	//PrintBTree(author);
	printf("\n��ȫ�����ߡ�\n");
	AoTuPrint(reader, 0);
	//PrintBTree(reader);
}


// ���ܱ�
void menu() {
	printf("\n\n");

	printf("                      �༶��2022���������3��                       \n");
	printf("                      �������»���                                  \n");
	printf("                      ѧ�ţ�3222004641                              \n");
	printf("                      ��Ŀ��ͼ����Ϣ����ϵͳ                        \n");

	printf("\n\n                    **��ѡ�����¹���**\n\n");
	printf("     |������������������������  &&   ���ܡ�������������������������|\n");
	printf("     |-������������������������������������������������������������|\n");
	printf("     |---------------------a    TO   ͼ�����----------------------|\n");
	printf("     |---------------------b    TO   ɾ��ͼ��----------------------|\n");
	printf("     |-������������������������������������������������������������|\n");
	printf("     |---------------------c    TO   ԤԼ����----------------------|\n");
	printf("     |---------------------d    TO   ����--------------------------|\n");
	printf("     |---------------------e    TO   ����--------------------------|\n");
	printf("     |-������������������������������������������������������������|\n");
	printf("     |---------------------f    TO   ��ѯͼ��----------------------|\n");
	printf("     |---------------------g    TO   ��ѯ����----------------------|\n");
	printf("     |---------------------h    TO   ��ѯ����----------------------|\n");
	printf("     |---------------------i    TO   �鿴ȫ������ӡB����-----------|\n");
	printf("     |-������������������������������������������������������������|\n");
	printf("     |---------------------j    TO   ������־----------------------|\n");
	printf("     |---------------------k    TO   �ļ��д洢��ͼ��--------------|\n");
	printf("     |---------------------l    TO   �˳�--------------------------|\n");
	printf("     |-������������������������������������������������������������|\n");
	printf("     |-������������������������������������������������������������|\n");
	printf("                                     ���������ѡ������ǵ� [ %d ] ��\n\n", nowDate);
}


// �򻯽���
void reactive() {
	printf("\n\n");
	system("PAUSE");
	system("cls");
	nowDate++;
	menu();
	printf("\n��ѡ�����������: ");
}

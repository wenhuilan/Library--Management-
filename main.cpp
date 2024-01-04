
#include "BTree.h"

int main() {
	char i;
	int k, test;
	BTree book = NULL;
	BTree author = NULL;
	BTree reader = NULL;
	LinkNode* Log = NULL;
	Result s;

	init(book, author, reader, Log);
	//
	menu();

	printf("\n��ѡ�����������: ");
	while ((i = getchar()) != 'l') {
		if (i == '\n')
			continue;
		if ('a' <= i && i <= 'k') {
			switch (i) {
				case'a':
						// ͼ�����
						addBook(book, author, Log);
					reactive();
					break;
				case'b':
						// ɾ��ͼ��
						deleteBook(book, author, Log);
					reactive();
					break;
				case'c':
						// ԤԼ����
						appointBook(book, reader, Log);
					reactive();
					break;
				case'd':
						// ����
						borrowBook(book, reader, Log);
					reactive();
					break;
				case'e':
						// ����
						backBook(book, reader, Log);
					reactive();
					break;
				case'f':
						// ��ѯͼ��
						bookData(book, Log);
					reactive();
					break;
				case'g':
					// �鿴����
					authorData(author, Log);
					reactive();
					break;
				case'h':
					// ��ѯ����
					readerData(reader, Log);
					reactive();
					break;
				case'i':
					// �鿴ȫ������ӡB����
					printBook(book, author, reader, Log);
					reactive();
					break;
				case'j':
					// ������־
					log(Log);
					reactive();
				case'k':
					// �鿴�ļ��д洢��ͼ����Ϣ
					readBookInfoFromFile("books.txt");
					reactive();
					break;
			}
		} else
			printf("\n��ѡ�������ѡ����������,������ѡ��:  ");

		while (getchar() != '\n')
			continue;
	}
	printf("\n\n�������������Ѿ���������л����ʹ�ã�\n\n");
	clearFile("books.txt");//����ļ�
	getchar();
}

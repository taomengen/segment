#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <ext/hash_map>
#include <iomanip>
#include <map>
#include <stdio.h>
#include <time.h>
#define MaxWordLength 21 // 最大词长字节（即4个汉字）

#define Separator " "    // 词界标记
#define UTF8_CN_LEN 3    // 汉字的UTF-8编码为3字节
using namespace std;

map<string, int> wordmap; // 词典

//计算词典
void compute_dict(void)
{
	string strtmp; //读取一行语料库
	ifstream dictfile("1998.txt.utf8");
	if (!dictfile.is_open())
	{
		cerr << "Unable to open 1998.txt.utf8" << endl;
		exit(-1);
	}
	while (getline(dictfile, strtmp) )
	{
		strtmp = strtmp.substr(strtmp.find(" ")+2);
		int p1 = 0;
		int p2 = 0;
		while(p2 < strtmp.size())
		{
			if(strtmp[p2] - 0x20 == 0 || strtmp[p2] == '\/' 
				|| (strtmp[p2] >= 65 && strtmp[p2] <= 90) 
				|| (strtmp[p2] >= 97 && strtmp[p2] <= 122)  
				|| strtmp[p2] == '-'
				|| strtmp[p2] == '{' || strtmp[p2] == '}'  
				|| (strtmp[p2] >= 48 && strtmp[p2] <= 57)
				|| strtmp[p2] == '[' || strtmp[p2] == ']' ) 
			{
				if(p2 > p1)
				{
					wordmap[strtmp.substr(p1, p2-p1)]++;
				}
				p2++;
				p1 = p2;
		
			}
			else
			{
				p2++;
			}
		}
	}
	dictfile.close();
}

//读入词典
void get_dict(void)
{
	string strtmp; 
	int num;
	string word;  
	
	ifstream dictfile("dict.txt.utf8");
	if (!dictfile.is_open())
	{
		cerr << "Unable to open dictput file: " << endl;
		exit(-1);
	}
	while (getline(dictfile, strtmp)) 
	{
		istringstream istr(strtmp);
		istr >> num;  
		istr >> word;  
		wordmap[word]++;
	}
	dictfile.close();
}

//删除语料库中的分词空格和词性标注符号
string eat_space(string s1)
{
	int p1 = 0;
	int p2 = 0;
	int count;
	string s2;
	while(p2 < s1.length())
	{
		if(s1[p2] - 0x20 == 0 || s1[p2] == '\/' ||  (s1[p2] >= 65 && s1[p2] <= 90) 
			|| (s1[p2] >= 97 && s1[p2] <= 122)  || s1[p2] == '-' || s1[p2] == '{' 
			|| s1[p2] == '}'  || (s1[p2] >= 48 && s1[p2] <= 57) ) 
		{
			if(p2 > p1)
			{
				s2 += s1.substr(p1, p2-p1);
			}
			p2++;
			p1 = p2;
		}
		else
		{
			p2++;
		}
	}
	return s2;
}

//用词典做正向最大匹配法分词
string forward_dict_segment(string s1)
{
	string s2 = ""; 
	while (!s1.empty()) 
	{
		int len = (int) s1.length(); 
		if (len > MaxWordLength)    
		{
			len = MaxWordLength;     
		}

		string w = s1.substr(0, len); 
		int n = (wordmap.find(w) != wordmap.end()); 
		while (len > UTF8_CN_LEN && n == 0) 
		{
			len -= UTF8_CN_LEN; 
			
			w = w.substr(0, len); 
		
			n = (wordmap.find(w) != wordmap.end());
		}
		
		s2 += w + Separator;

		s1 = s1.substr(w.length(), s1.length()); 	
	}

	s2 = s2.substr(0, s2.length()-1);
	return s2;
}

//用词典做逆向最大匹配法分词
string reverse_dict_segment(string s1)
{
	string s2 = ""; 
	while (!s1.empty()) 
	{
		int len = (int) s1.length(); 
		if (len > MaxWordLength)     
		{
			len = MaxWordLength;     
		}
	
		string w = s1.substr(s1.length() - len, len); 	
		int n = (wordmap.find(w) != wordmap.end());
		while (len > UTF8_CN_LEN && n == 0) 
		{
			len -= UTF8_CN_LEN; 
			w = s1.substr(s1.length()-len, len); 	
			n = (wordmap.find(w) != wordmap.end());
		}
		w = w + Separator; 
		s2 = w + s2; 
		s1 = s1.substr(0, s1.length()-len); 
	}

	s2 = s2.substr(0, s2.length()-1);
	return s2;
}

//双向最大匹配法
string dict_segment(string str)
{
	string str1 = forward_dict_segment(str);
	string str2 = reverse_dict_segment(str);
	int numspace1 = 0;
	int numspace2 = 0;
	for (int i = 0; i < str1.size(); i++)
	{
		if (str1[i] == ' ')
		{
			numspace1++;
		}
	} 
	for (int i = 0; i < str2.size(); i++)
	{
		if (str2[i] == ' ')
		{
			numspace2++;
		}
	}
	return numspace1 > numspace2 ? str1 : str2; 
}
//在执行中文分词前，过滤半角空格以及其他非 UTF-8 字符
string seg_analysis(string s1)
{
	string s2;
	string s3 = "";
	int p1 = 0;
	int p2 = 0;
	int count;
	while(p2 < s1.length())
	{
		if(((s1[p2]>>4)&0xe) ^ 0xe)
		{
			//过滤非 utf-8 字符
			count = 0;
			do
			{
				p2++;
				count++;
			}while((((s1[p2]>>4)&0xe) ^ 0xe) && p2 < s1.length());
			//特殊字符前的串
			s2 = s1.substr(p1,p2-count-p1);
			//特殊字符，不要用汉字分词处理
			s3 += dict_segment(s2) + s1.substr(p2-count,count);
			if(p2 <= s1.length())
			{
				//剩余字符串
				s1 = s1.substr(p2,s1.length()-p2);
			}
			p1 = p2 = 0;
		}
		else
			p2 += UTF8_CN_LEN;
	}
	if(p2 != 0)
	{
		s3 += dict_segment(s1);		
	}
	return s3;
};

int main(int argc, char* argv[])
{
	cout << "Compute dict from text,please input 1:" << endl;
	cout << "Get the existed dict,  please input 2:" << endl;
	int num;
	cin >> num;
	clock_t start, finish;
	double duration;
	start = clock();

	if (num == 1)
	{
		compute_dict();
		finish = clock();
		duration = (double)(finish - start) / CLOCKS_PER_SEC;
		cout << "词典计算完毕，耗时 " << duration << " s" << endl;
	}
	else if (num == 2)
	{
		get_dict();
		finish = clock();
		duration = (double)(finish - start) / CLOCKS_PER_SEC;
		cout << "词典读入完毕，耗时 " << duration << " s" << endl;
	}
	else
	{
		cout << "error input!" << endl;
		exit(-1);
	}
	
	string strtmp; 
	string line;   

	// 打开语料库文件
	ifstream infile("1998.txt.utf8"); 
	if (!infile.is_open())            
	{
		cerr << "Unable to open 1998.txt.utf8: "<< endl;
		exit(-1);
	}

	//保存分词后的文本
	ofstream outfile1("result.txt.utf8"); 
	if (!outfile1.is_open()) 
	{
		cerr << "Unable to open result.txt.utf8" << endl;
		exit(-1);
	}

	//保存分词前的文本
	ofstream outfile2("pre.1998.txt.utf8"); 
	if (!outfile2.is_open()) 
	{
		cerr << "Unable to open pre.1998.txt.utf8" << endl;
		exit(-1);
	}

	start = clock();
	cout << "正在分词并输出到文件，请稍候..." << endl;

	//读入语料库中的每一行，先恢复成原来文本，再用双向最大匹配法分词
	while (getline(infile, strtmp)) 
	{
		line = eat_space(strtmp);
		outfile2 << line << endl;

		line = seg_analysis(line); 
		outfile1 << line << endl;  
	}
	
	infile.close();
	outfile1.close();
	outfile2.close();

	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	cout << "分词完毕，耗时 " << duration << "　s" << endl;
	cout << "分词结果保存在 result.txt.utf8中。" << endl;

	exit(0);
}
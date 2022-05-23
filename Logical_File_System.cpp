#include <iostream>
#include <string>
#include <algorithm>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <string.h>
#include <cstring>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <fcntl.h> 
#include <sys/ioctl.h>
#include<sys/mman.h>
#include<fcntl.h>
#include <signal.h>
#include<fstream>
using namespace std;


class Data_Track
{
public:
	char* file_name;
	int starting_block;
	int No_of_blocks;
	Data_Track(){
		file_name=new char[30];
		starting_block=0;
		No_of_blocks=0;
	}
	Data_Track(char* fname,int st,int n)
	{
		file_name=fname;
		starting_block=st;
		No_of_blocks=n;
	}
};

class FAT
{
public:
	bool *free_block; // No of free blocks
	Data_Track * dt;
	int no_of_files;
	FAT()
	{
		dt=new Data_Track[1000]; ///max files in our logical file system
		free_block=new bool[1000];
		for(int i=0;i<1000;i++)
		{
			free_block[i]=true;
		}
		no_of_files=0;
	}
};

class FCB
{
public:
	char* file_name;
	string writing_access;
	string reading_access;
	int file_size;
	char ** data_block_pointer;
	FCB()
	{
		file_name=new char[30];
		
	}
};
class File
{
public:
	char* file_name;
	FCB fcb;
	File()
	{
		file_name=new char[30];
		fcb.writing_access='\0';
		fcb.reading_access='\0';
		fcb.file_name='\0';
		
	}
};
class Directory
{
public:
	string name;
	int file_count;
	int directory_count;
	File *files;
	Directory *sub_directories;
	void Set_directory(string n)
	{
		name=n;
		file_count=0;
		directory_count=0;
		files=new File[10];
		sub_directories=new Directory[10];
	}
};

class Directory_Tree
{
public:
	
	Directory *directories;
	Directory_Tree()
	{
		directories=new Directory();
	}
	bool compareString(string a,string b)
	{
		int length1=a.size();
		int length2=b.size();
		if(length1==length2)
		{
			int i=0;
			while(a[i]!='\0'&&b[i]!='\0')
			{
				if(a[i]!=b[i])
					return false;
				i++;
			}
			return true;
		}
		return false;
	}
	bool is_directory_exist(string path)
	{
		if(path=="" || !directories )
			return false;
		string temp="";
		int i=0;
		while(path[i]!=':')
		{
			temp=temp+path[i];
			i++;
		}
		if(!compareString(temp,directories->name))
			return false;
		i++;
		if(path[i]!='\\')
			return false;
		i=i+1;
		temp="";
		while(path[i]!='\0')
		{
			if(path[i]=='\\')
			{
				if(!directories->sub_directories)
					return false;
				int j=0;
				bool flag=false;
				while(j<10&&flag==false)
				{
					if(compareString(temp,directories->sub_directories[j].name))
					{
						directories[0]=directories->sub_directories[j];
						flag=true;
					}
					if(flag==false)
						return false;
					j++;
					
				}
				temp="";
				i++;				
			}
			else
				temp=temp+path[i];	
			i++;
		}
		return true;		
	}
};

class VCB
{
public:
	Directory_Tree directory_tree;
	FAT fat;
};


class VHD
{
public:
	string name;
	VCB vcb;
	char **data_block;
	int data_block_size;
	VHD(string n,int size)
	{
		name=n;
		data_block_size=size;
		data_block = new char*[1000];
		for(int i=0;i<1000;i++)
		{
			data_block[i]=new char[size];
		}
	}
};
void ls(Directory directory)
{
	
	for(int i=0;i<directory.file_count;i++){
		cout<<directory.files[i].file_name<<"\t";
	}
	for(int i=0;i<directory.directory_count;i++){
		cout<<directory.sub_directories[i].name<<"\t";
	}
	cout<<endl;
}

void make_directory(Directory & curr_dir,string dn)
{
	for(int i=0;i<curr_dir.directory_count;i++)
	{
		if(curr_dir.sub_directories[curr_dir.directory_count].name==dn)
		{
			cout<<"Sub Directory is already exist in this Directory so change the name of new Directory:\n";
			return;
		}
	}
	curr_dir.sub_directories[curr_dir.directory_count].name=dn;
	curr_dir.sub_directories[curr_dir.directory_count].file_count=0;
	curr_dir.sub_directories[curr_dir.directory_count].files=new File[10]; //Suppose one sub_dir have max 10 files
	curr_dir.sub_directories[curr_dir.directory_count].sub_directories=new Directory[10];
	curr_dir.directory_count++;
}
void remove(Directory & curr_dir,int i,VHD & vhd)
{
	int j=0;
	while(j<curr_dir.file_count)
	{
		curr_dir.files[j].file_name='\0';
		curr_dir.files[j].fcb.data_block_pointer='\0';
		int k=0;
		while(curr_dir.files[j].file_name!=vhd.vcb.fat.dt[k].file_name && k<vhd.vcb.fat.no_of_files)k++;
		//if(k==no_of_file-1&&curr_dir.file[j].name!=vhd.fat.dt[k].file_name)// no need bcz file must exist in fat...
		vhd.vcb.fat.dt[k].file_name='\0';
		int start=vhd.vcb.fat.dt[k].starting_block;
		for(int l=0;l<vhd.vcb.fat.dt[k].No_of_blocks;l++)
		{
			vhd.vcb.fat.free_block[l]=true;
			vhd.data_block[l]='\0';
		}
		vhd.vcb.fat.no_of_files--;
		j++;
	}
	curr_dir.file_count=0;
	//recursion for sub directories
	if(curr_dir.directory_count==0 || i >=10) //10 max sub directories can exists in a directory 
	{
		return;
	}
	else
	{
		remove(curr_dir.sub_directories[i],i,vhd);
		i++;
	}
	
}
void remove_directory(Directory & curr_dir,string name,VHD & vhd)
{
	if(curr_dir.directory_count==0)
	{
		cout<<"No directory exists here:\n";
		return;
	}
	else
	{
		int i=0;
		while(i<curr_dir.directory_count&&name!=curr_dir.sub_directories[i].name)i++;
		if(i==curr_dir.directory_count&&name!=curr_dir.sub_directories[i].name)
		{
			cout<<"Directory did not exists:\n";
		}
		else if(name==curr_dir.sub_directories[curr_dir.directory_count-1].name)
		{
			remove(curr_dir.sub_directories[curr_dir.directory_count-1],0,vhd);
			curr_dir.directory_count--;
		}
		else
		{
			remove(curr_dir.sub_directories[i],0,vhd);
			while(i<curr_dir.directory_count-1)
			{
				curr_dir.sub_directories[i]=curr_dir.sub_directories[i+1];
				curr_dir.directory_count--;
				i++;
			}	
		}
	}

}

void cd(Directory & curr_dir,string name,VHD vhd)
{
	if(name!="~")
	{
		if(curr_dir.directory_count==0)
		{
			cout<<"No sub directory exists here:\n";
			return;
		}
		else
		{
			int i=0;
			while(i<curr_dir.directory_count&&name!=curr_dir.sub_directories[i].name)i++;
			if(i==curr_dir.directory_count&&name!=curr_dir.sub_directories[i].name)
			{
				cout<<"Directory did not exists:\n";
				return;
			}
		
			else
			{
				cout<<"Now Current Working Directory is :\n"<<curr_dir.sub_directories[i].name<<endl;
				curr_dir=curr_dir.sub_directories[i];
			
			}
		}
	}
	else if(name=="~")
	{
		curr_dir=vhd.vcb.directory_tree.directories[0];
		cout<<"Now Current Working Directory is :\n"<<curr_dir.name<<endl;
				
	}
}


void import(VHD & vhd,Directory & source,char* file)
{
	int fd;
	fd=open(file,O_RDWR);
	if(fd==-1)
	{
		cout<<"File could not open:\n";
		exit(1);
	}
	void * data;
	data=mmap(NULL,getpagesize(),PROT_READ,MAP_SHARED,fd,0);
	char * d=(char *)data;
	ifstream in_file(file, ios::binary);
	in_file.seekg(0, ios::end);
	int file_size = in_file.tellg();
	int l=0;
	int size=file_size/vhd.data_block_size;
	int reminder=file_size%vhd.data_block_size;
	if(reminder>0)
	{
		size=size+1;
	}
	int no_of_file=vhd.vcb.fat.no_of_files;
	int file_count=source.file_count;
	vhd.vcb.fat.dt[vhd.vcb.fat.no_of_files].No_of_blocks=size;  /// size = no of block required to store data
	while(vhd.vcb.fat.free_block[l]==false)l++;

	vhd.vcb.fat.dt[no_of_file].starting_block=l;
	vhd.vcb.fat.dt[no_of_file].file_name=file; 
	source.files[file_count].fcb.data_block_pointer=new char*[size];

	int k=0;
	for(int i=0;i<size;i++)     //////// Assign data of file to data_block
	{
		vhd.vcb.fat.free_block[l]=false;
		l++;
		for(int j=0;j<vhd.data_block_size && d[j];j++)
		{
			vhd.data_block[l][j]=d[k];  
			k++;
		}
		source.files[file_count].fcb.data_block_pointer[i]=vhd.data_block[l];
	}      
	vhd.vcb.fat.no_of_files++;
	source.files[file_count].file_name=file; 
	source.files[file_count].fcb.file_name=file;
	source.files[file_count].fcb.reading_access="O_RDONLY";    /// how to find from system existing file
	source.files[file_count].fcb.writing_access="O_WRONLY";   //// how to find from system existing file
	
	source.files[file_count].fcb.file_size=file_size;
	source.file_count++;
	
}


int main()
{
	int size=0,i=0;
	cout<<"Enter block size of virtual hard disk:\t";
	cin>>size;
	char* name=new char[30];
	cout<<"Enter the name of new virtual hard drive:\t";	
	cin>>name;
	string path;
	cout<<"Enter the path as //<root dir>:\t";
	cin>>path;
	
	VHD vhd_obj(name,size);
	
	VCB obj;
	obj.directory_tree.directories[0].Set_directory("Assignment4"); ///root dir
	make_directory(obj.directory_tree.directories[0],"riasat");
	make_directory(obj.directory_tree.directories[0],"khan");
	
	
	vhd_obj.vcb=obj;
	string command;
	int x=0;
	while(x!=-1)
	{
		cout<<"Enter the 'ls' to run ls_command\nEnter 'mkdir' to make a new directory\nEnter 'rmdir' to remove a directory\nEnter 'cd' to change directory\nEnter 'import' to import a file from your System:\n";
		cout<<"Enter the command:\t";
		cin>>command;
		if(command=="ls"){
			ls(vhd_obj.vcb.directory_tree.directories[0]);
		}
		else if(command=="mkdir"){
			string dir_name;
			cout<<"Enter Name of directoru to create:\t";
			cin>>dir_name;
			make_directory(vhd_obj.vcb.directory_tree.directories[0],dir_name);
		}
	
		else if(command=="rmdir"){
			char* rm_dir_name=new char[20];
			cout<<"Enter Name of directoru to remove:\t";
			cin>>rm_dir_name;
			remove_directory(vhd_obj.vcb.directory_tree.directories[0],rm_dir_name,vhd_obj);
		}
		else if(command=="cd")	{
			string new_dir;
			cout<<"Enter dir name to move to directory:\t";
			cin>>new_dir;
			cd(vhd_obj.vcb.directory_tree.directories[0],new_dir,vhd_obj);
		}
		else if(command=="import"){
			char *import_dir=new char[30];
			cout<<"Enter dir name to import:\t";
			cin>>import_dir;
			import(vhd_obj,obj.directory_tree.directories[0],import_dir);
		}
		else {
			cout<<"You Entered Wrong command:\n";
		}
		cout<<"Enter -1 to terminate prograam :\n";
		cin>>x;
	}
	return 0;
}


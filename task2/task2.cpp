#include <iostream> 
#include <fstream>  
#include <string> 
#include <vector>
#include <functional>
#include <map>
#include <pthread.h>
#include <semaphore.h>

using namespace std; 

//Declaring variables
int pageId;
string readWrite;
string pageContents; 

int clientThreads; 
int finished = 0;
int requestNo = 1;
int threadId; 

sem_t clientSemaphore [10]; 
sem_t serverSemaphore; 

map<int, string> memoryBuffer; 

//split helper method
void split(const string& s, char c, vector<string>& v) 
{
   string::size_type i = 0;
   string::size_type j = s.find(c);

   while (j != string::npos) 
   {
      v.push_back(s.substr(i, j-i));
      i = ++j;
      j = s.find(c, j);

      if (j == string::npos)
         v.push_back(s.substr(i, s.length()));
   }
}

//Client Thread
void * func(void *arg) 
{
	string line;
	int threadRequestNo;
	
	//Each client thread should keep a log of the pages that it has read 
	//inside a file called “client_log_n” where n is its thread id.
	ofstream clientFile("client_log_" + to_string(((int)(long)arg)) + ".dat");
	
	//Each thread reads from its own client_request_n.dat file to get the page 
	//read or write requests.
	ifstream requestFile("client_requests_" + to_string(((int)(long)arg)) + ".dat");
	
	if (requestFile.is_open()) 
	{
		while (getline(requestFile, line)) 
		{
			vector<string> v;
			split(line, ' ', v);
			
			threadRequestNo = stoi(v[0]);
			
			//The server must process the requests in ascending order according 
			//to request number specified in the client_requests_n.dat files
			while(requestNo != threadRequestNo)
			{
				sem_wait(&clientSemaphore[((int)(long)arg)]);
			}
			
			threadId = ((int)(long)arg);
						
			readWrite = v[1];
			pageId = stoi(v[2]);
			
			if (readWrite.compare("read") == 0) 
			{ 
				sem_post(&serverSemaphore);
				sem_wait(&clientSemaphore[threadId]);
				clientFile << pageId << " " <<  memoryBuffer[pageId] << endl;
				
				requestNo++;
				
				for (int i = 0; i < clientThreads; i++) 
				{
					sem_post(&clientSemaphore[i]); 
				}
			}
			else 
			{
				pageContents = v[3];
				sem_post(&serverSemaphore);
				sem_wait(&clientSemaphore[threadId]);
			}
			
			for (int i = 0; i < clientThreads; i++) 
			{
				sem_post(&clientSemaphore[i]); 
			}
			
		} 
		
		requestFile.close(); 
		
		clientFile.close(); 
		
		finished++;
		
		//When all the clients finish their requests the server thread must 
		//end and cause the entire process to end. 
		if (finished == clientThreads) 
		{
			sem_post(&serverSemaphore);
		}
		
	} 
	else 
	{	
		cout << "The file all_requests.dat could not be opened."; 
	}
	
}	

//Server Thread    
int main (int argc, char* argv[]) 
{
	string line;
	
	//The server thread starts and reads in the init_buffer_pages.dat file to 
	//initialise the memory buffer. 
	
	//Open buffer file
	ifstream bufferFile("init_buffer_pages.dat"); 

	if(bufferFile.is_open()) 
	{
		//Map data from buffer file
		while (getline(bufferFile, line))
		{
			vector<string> v;
			split(line, ' ', v);
			
			pageId = stoi(v[0]);
			pageContents = v[1];
			
			memoryBuffer[pageId] = pageContents; 
		}
		
		//Close buffer file
		bufferFile.close(); 
	}
	else
	{
		cout << "The file init_buffer_pages.dat could not be opened."; 
	}
	
	//Create N client threads, where the number N is taken from the command line.
	clientThreads = *argv[1] - 48; 
	pthread_t threads[clientThreads]; 

	for(int i = 0; i < clientThreads; i++)
	{
		pthread_create(&threads[i], NULL, func, (void *)(long)(i)); 
	}  
	
	sem_wait(&serverSemaphore); 
	
	while (clientThreads != finished)
	{
        if (readWrite.compare("read") == 0)
        {
            pageContents = memoryBuffer[pageId];
			sem_post(&clientSemaphore[threadId]);
		} 
		else
		{
			memoryBuffer[pageId] = pageContents;
			
			requestNo++;
			
			for (int i = 0; i < clientThreads; i++) 
			{
				sem_post(&clientSemaphore[i]); 
			}
		}
		
		sem_wait(&serverSemaphore); 
    } 
	
	//Join Threads
	for (int i = 0; i < clientThreads; i++) 
	{
		pthread_join(threads[i], NULL); 
	}
	return 0;
}
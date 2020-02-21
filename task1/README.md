# Mulithreaded Memory Manager
The tasks in this repository are to create a memory manager that manages page reads and writes from multiple client threads.  In the first programming task the memory manager keeps all the pages in a memory buffer large enough to fit all the pages and the server processes a list of requests and sends them to clients to log. In the second task the solution from task 1 is extended so that requests are sent from the client to the server instead of all the requests originating in the server.

The command line below is used to compile each directory respectively:
```
g++ -pthread -g -o  task1 task1.cpp -std=c++11
```

## Task 1

### Specifications
There is 1 server thread and N client threads, where N is supplied by the user as a command line argument. The server opens a file called *“all_requests.dat”*, the file has the following format:
```
<client id><space><read/write><space><page id><space><contents of page>
<client id><space><read/write><space><page id><space><contents of page>
...
```

**client id** – refers to the id of the client thread which is between 0 and N – 1.  Note this is not the thread id that is assigned by the system.

**read/write** – refers to whether the request is a read or write request.

**page id** – refers to id of the page being requested.

**contents of page** – this field only exists if the request is a write request.  It contains the contents of a page that is to be written into the server which is a sequence of alphabet characters (a-z, A-Z) with no spaces or anything else in between and has a maximum size of 4096 characters (size of page for most machines).

The server thread also reads from the file called *“init_buffer_pages.dat”* which contains the initial contents of ALL the buffer pages (you can be sure read and write requests are to pages existing in this file).  It has the following format:
```
<page id><space><contents of the page>
<page id><space><contents of the page>
...
```
### Operation
* The server thread starts and reads in the *init_buffer_pages.dat* file to initialise the memory buffer. 
* Create N client threads, where the number N is taken from the command line.
* The server thread reads from *all_requests.dat* file to get the page read or write requests.  The server must process these requests in order from the 1st to the last.  For example the 2nd request on the list must be processed before the 3rd request.
* Each client thread should keep a log of the pages that it has read inside a file called *“client_log_n”* where n is its thread id.   The pages read must be written into the log file in the order that they are received from the server.  Please note you only need to log read requests.  In the case of a write request the server does not need to contact any of the clients.    This will let us know if server is processing the requests in the correct order.  The beginning of each entry should contain the page id followed by the page contents.  
* Please note that the server must wait for the current client thread to have finished writing its log entry before getting the next thread to log its read request.
* When all the clients finish their requests the server thread must end and cause the entire process to end.

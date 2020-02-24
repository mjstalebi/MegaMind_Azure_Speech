//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.md file in the project root for full license information.
//

// <code>
#include <iostream> // cin, cout
#include <speechapi_cxx.h>

//MegaMind
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "sock.h"

#include <string>
#include <algorithm>

using namespace std;
using namespace Microsoft::CognitiveServices::Speech;
void report(const char* msg, int terminate) {
  perror(msg);
  if (terminate) exit(-1); /* failure */
}
void wait_for_keyword_detection(){
   int option = 1;
  int fd = socket(AF_INET,     /* network versus AF_LOCAL */
		  SOCK_STREAM, /* reliable, bidirectional: TCP */
		  0);          /* system picks underlying protocol */
  if (fd < 0) report("socket", 1); /* terminate */
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  	
  /* bind the server's local address in memory */
  struct sockaddr_in saddr;
  memset(&saddr, 0, sizeof(saddr));          /* clear the bytes */
  saddr.sin_family = AF_INET;                /* versus AF_LOCAL */
  saddr.sin_addr.s_addr = htonl(INADDR_ANY); /* host-to-network endian */
  saddr.sin_port = htons(PortNumber_start_speech_1);        /* for listening */
  
  if (bind(fd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0)
    report("bind", 1); /* terminate */
	
  /* listen to the socket */
  if (listen(fd, MaxConnects) < 0) /* listen for clients, up to MaxConnects */
    report("listen", 1); /* terminate */

  fprintf(stderr, "Listening on port %i for clients...\n", PortNumber_start_speech_1);
  /* a server traditionally listens indefinitely */
   struct sockaddr_in caddr; /* client address */
   unsigned int len = sizeof(caddr);  /* address length could change */
   
   int client_fd = accept(fd, (struct sockaddr*) &caddr, &len);  /* accept blocks */
   if (client_fd < 0) {
     report("accept", 0); /* don't terminated, though there's a problem */
   }

   /* read from client */
   int i;
   char buffer[BuffSize + 1];
   memset(buffer, '\0', sizeof(buffer)); 
   cout<<"before read\n";
   int count = read(client_fd, buffer, sizeof(buffer));
   cout<<"after read\n";
   if (count > 0) {
       cout<<" ... \n";
   }
   close(client_fd); /* break connection */
   close(fd);
   return;
}

void send_cmd_to_sdk(std::string cmd){

  /* fd for the socket */
  int sockfd = socket(AF_INET,      /* versus AF_LOCAL */
		      SOCK_STREAM,  /* reliable, bidirectional */
		      0);           /* system picks protocol (TCP) */
  if (sockfd < 0) report("socket", 1); /* terminate */

  /* get the address of the host */
  struct hostent* hptr = gethostbyname(Host); /* localhost: 127.0.0.1 */ 
  if (!hptr) report("gethostbyname", 1); /* is hptr NULL? */
  if (hptr->h_addrtype != AF_INET)       /* versus AF_LOCAL */
    report("bad address family", 1);
  
  /* connect to the server: configure server's address 1st */
  struct sockaddr_in saddr;
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = 
     ((struct in_addr*) hptr->h_addr_list[0])->s_addr;
  saddr.sin_port = htons(PortNumber_end_speech_1); /* port number in big-endian */
  
  if (connect(sockfd, (struct sockaddr*) &saddr, sizeof(saddr)) < 0)
    report("connect", 1);
  
  /* Write some stuff and read the echoes. */ 
  if (write(sockfd, cmd.c_str(), strlen(cmd.c_str())+1) > 0) {
    /* get confirmation echoed from server and print */
    cout<<"successful write\n";
  }
  puts("Client done, about to exit...");
  close(sockfd); /* close the connection */
   return;
}

std::string recognizeSpeech() {
    // Creates an instance of a speech config with specified subscription key and service region.
    // Replace with your own subscription key and service region (e.g., "westus").
    auto config = SpeechConfig::FromSubscription("XXX", "XXX");

    // Creates a speech recognizer
    auto recognizer = SpeechRecognizer::FromConfig(config);
    cout << "Say something...\n";

    // Starts speech recognition, and returns after a single utterance is recognized. The end of a
    // single utterance is determined by listening for silence at the end or until a maximum of 15
    // seconds of audio is processed.  The task returns the recognition text as result. 
    // Note: Since RecognizeOnceAsync() returns only a single utterance, it is suitable only for single
    // shot recognition like command or query. 
    // For long-running multi-utterance recognition, use StartContinuousRecognitionAsync() instead.
    auto result = recognizer->RecognizeOnceAsync().get();

    // Checks result.
    if (result->Reason == ResultReason::RecognizedSpeech) {
        cout << "We recognized: " << result->Text << std::endl;
	return result->Text;
    }
    else if (result->Reason == ResultReason::NoMatch) {
        cout << "NOMATCH: Speech could not be recognized." << std::endl;
        return "stop";
    }
    else if (result->Reason == ResultReason::Canceled) {
        auto cancellation = CancellationDetails::FromResult(result);
        cout << "CANCELED: Reason=" << (int)cancellation->Reason << std::endl;

        if (cancellation->Reason == CancellationReason::Error) {
            cout << "CANCELED: ErrorCode= " << (int)cancellation->ErrorCode << std::endl;
            cout << "CANCELED: ErrorDetails=" << cancellation->ErrorDetails << std::endl;
            cout << "CANCELED: Did you update the subscription info?" << std::endl;
        }
        return "stop";
    }
}

int main(int argc, char **argv) {
    setlocale(LC_ALL, "");
    while(1){
        wait_for_keyword_detection();
	std::string cmd; 
    	cmd = recognizeSpeech();
        cmd.erase(remove(cmd.begin(), cmd.end(), '\n'), cmd.end());
	cout << "your command is "<<cmd;
	send_cmd_to_sdk(cmd);
    }

    return 0;

}
// </code>

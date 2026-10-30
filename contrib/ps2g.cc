#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

#include <stream.h>
#include <fstream.h>
#include <String.h>

void sigchld (int)
{
 int status, pid;
 pid = wait(&status);
}

main(int argc, char **argv)
{
 String outFile("out.g"); 
 FILE *fd= stdin;
 char buf[1024];
     
 if(argc > 1)
  {
   outFile = String(argv[1]) + ".g";
   fd = fopen (argv[1], "r");
  }
 else
  {
   if(putenv("UNIX95=XPG4"))
    {
     cerr << "Couldn't set environment\n";
     exit(-1);
    }
   signal (SIGCHLD, sigchld);
   pid_t pid;
   int pipefd[2];
   pipe (pipefd);
   
   if ((pid = fork()) == (pid_t)0) 
    {
     close(1);      /* close stdout */
     dup (pipefd[1]);
     close (pipefd[0]);
     execl ("/bin/ps", "ps", "-e", "-o", "user pid ppid comm nice vsz", 
            (char *)0);
    }
   else 
    {
     close(0); /* close stdin  */
     dup (pipefd[0]);
     close (pipefd[1]);
     fd = fdopen(0, "r");
    }
  }
 
 ofstream ofDatei((const char *)outFile);
 if(!ofDatei.good())
  {
   cout << "Couldn't open File: " << outFile << endl;
   exit(-1);
  }
  
 ofDatei << "GRAPH \"PS Output\" = UNDIRECTED\n{$\nFF_SC 50\n$}"
         << endl;
 fgets(buf, 1024, fd);
 
 while(fgets(buf, 1024, fd))
  {
   String zeile(buf);
   int felderAnz = zeile.freq(" ");
   String felder[felderAnz+2];
   split(zeile, felder, felderAnz+2, RXwhite);

   // Schreibe Knoten

   ofDatei << felder[1] << " {$ ";
   
   int niceVal = atoi(felder[4]);
   if(niceVal < 20)
    ofDatei << " FF_C \"red\" ";
   else if(niceVal > 20)
    ofDatei << " FF_C \"blue\" ";
   else 
    ofDatei << " FF_C \"black\" ";
   
   if(atoi(felder[1]) == 0)
    ofDatei << " SN ";
   
   ofDatei << " FF_R " << dec((int)(sqrt(atoi(felder[5])/100)+10));
   
   ofDatei << " $} \"" << felder[0] << ":\n" 
           << felder[3] << "\"\n";
   
   // Schreibe Kante
   
   ofDatei << " " << felder[2] << " {$ FF_C \"grey\" ";
   ofDatei << "$} \"\";" << endl;
  }
 ofDatei << "END" << endl;
 ofDatei.flush();
}

/*
;;; Local Variables: ***
;;; compile-command:"c++ ps2g.cc -o ps2g -lm -lg++" ***
;;; End: ***
*/

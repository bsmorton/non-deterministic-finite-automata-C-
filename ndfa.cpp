// Submitter: bsmorton(Morton, Bradley)
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "ics46goody.hpp"
#include "array_queue.hpp"
#include "array_priority_queue.hpp"
#include "array_set.hpp"
#include "array_map.hpp"


typedef ics::ArraySet<std::string>                     States;
typedef ics::ArrayQueue<std::string>                   InputsQueue;
typedef ics::ArrayMap<std::string,States>              InputStatesMap;

typedef ics::ArrayMap<std::string,InputStatesMap>       NDFA;
typedef ics::pair<std::string,InputStatesMap>           NDFAEntry;

bool gt_NDFAEntry (const NDFAEntry& a, const NDFAEntry& b)
{return a.first<b.first;}

typedef ics::ArrayPriorityQueue<NDFAEntry,gt_NDFAEntry> NDFAPQ;

typedef ics::pair<std::string,States>                   Transitions;
typedef ics::ArrayQueue<Transitions>                    TransitionsQueue;


//Read an open file describing the non-deterministic finite automaton (each
//  line starts with a state name followed by pairs of transitions from that
//  state: (input followed by a new state, all separated by semicolons), and
//  return a Map whose keys are states and whose associated values are another
//  Map with each input in that state (keys) and the resulting set of states it
//  can lead to.
const NDFA read_ndfa(std::ifstream &file) {
    NDFA ndfa;
    std::string line;
    while(std::getline(file,line)){
        std::vector<std::string> temp=ics::split(line,";");
        ics::ArrayMap<std::string,ics::ArraySet<std::string>> tempmap;
        std::string state=temp[0];
        ndfa.put(state,tempmap);
        for(int i=1; i < temp.size(); i++){
            if(i%2==1){
                if(ndfa[state].has_key(temp[i])==0){
                    ics::ArraySet<std::string> tempset;
                    ndfa[state].put(temp[i],tempset);
                    ndfa[state][temp[i]].insert(temp[i+1]);

                }
                else{
                    ndfa[state][temp[i]].insert(temp[i+1]);
                }

            }

        }

    }
    return ndfa;
}


//Print a label and all the entries in the finite automaton Map, in
//  alphabetical order of the states: each line has a state, the text
//  "transitions:" and the Map of its transitions.

void print_ndfa(const NDFA& ndfa) {
    ics::ArrayPriorityQueue<NDFAEntry,gt_NDFAEntry> q;
    q.enqueue_all(ndfa);
    std::cout << "The Non-Deterministic Finite Automaton's Description" << std::endl;
    for(NDFAEntry& n : q){
        std::cout << "  " << n.first << " transitions: " << n.second << std::endl;
    }

}


//Return a queue of the calculated transition pairs, based on the non-deterministic
//  finite automaton, initial state, and queue of inputs; each pair in the returned
//  queue is of the form: input, set of new states.
//The first pair contains "" as the input and the initial state.
//If any input i is illegal (does not lead to any state in the non-deterministic finite
//  automaton), ignore it.
TransitionsQueue process(const NDFA& ndfa, std::string state, const InputsQueue& inputs) {
    TransitionsQueue tq;
    Transitions t;
    t.second.insert(state);
    tq.enqueue(t);
    ics::ArraySet<std::string> possible;
    possible.insert(state);
    for (std::string& s: inputs) {
        Transitions temp;
        for (std::string & str : possible) {
            if(ndfa.has_key(str)){
                if(ndfa[str].has_key(s)==1){
                    temp.first=s;
                    temp.second.insert_all(ndfa[str][s]);
                }
            }
        }
        tq.enqueue(temp);
        possible=temp.second;
    }
    return tq;
}


//Print a TransitionsQueue (the result of calling process) in a nice form.
//Print the Start state on the first line; then print each input and the
//  resulting new states indented on subsequent lines; on the last line, print
//  the Stop state.
void interpret(TransitionsQueue& tq) {  //or TransitionsQueue or TransitionsQueue&&
    Transitions t=tq.dequeue();
    std::cout << "Start state = " << t.second << std::endl;
    while(tq.size()>0){
        t=tq.dequeue();
        std::cout << "  Input = " << t.first << "; new states = " << t.second << std::endl;
    }
    std::cout << "Stop state(s) = " << t.second << std::endl;

}



//Prompt the user for a file, create a finite automaton Map, and print it.
//Prompt the user for a file containing any number of simulation descriptions
//  for the finite automaton to process, one description per line; each
//  description contains a start state followed by its inputs, all separated by
//  semicolons.
//Repeatedly read a description, print that description, put each input in a
//  Queue, process the Queue and print the results in a nice form.
int main() {
  try {
      std::string file1;
      std::cout << "Enter a non-deterministic finite automaton's file: ";
      std::getline(std::cin,file1);
      std::ifstream g1(file1);
      NDFA ndfa=read_ndfa(g1);
      print_ndfa(ndfa);
      std::cout << std::endl;
      std::string file2;
      std::cout << "Enter the name of a file with the start-states and input: ";
      std::getline(std::cin,file2);
      std::ifstream g2(file2);
      std::cout << std::endl;
      std::string line;
      while(std::getline(g2,line)){
          std::cout << "Starting new simulation with description: " << line << std::endl;
          ics::ArrayQueue<std::string> q;
          std::vector<std::string> temp=ics::split(line,";");
          q.enqueue_all(temp);
          TransitionsQueue t= process(ndfa,q.dequeue(),q);
          interpret(t);
          std::cout << std::endl;
      }




  } catch (ics::IcsError& e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}

// Jesse Runner
// Homework 5 - Distributed, Parallel & Web-Centric Computing
// This project takes in a dictionary and an encrypted password file as
// command line arguments. It then uses the salts in the encrypted file
// to brute force find the real passwords. It uses the MPI library to
// distribute the work amongst X number of proccesses as specified at runtime 

#define _XOPEN_SOURCE
#include<iostream>
#include<unistd.h>
#include<vector>
#include<string>
#include<fstream>
#include<mpi.h>
using namespace std;

// Function: read_dict
// Parameters: string filename
// Return type: vector<string>
// Description: This function takes in a filename and opens that file,
// reading in all dictionary words used for this program and storing them into
// a vector
vector<string> read_dict (string filename);

// Function: read_encrypt
// Parameters: string filename
// Return type: vector<string>
// Description: This function takes in a filename and opens that file,
// reading in all encrypted passwords used for this program and storing them into
// a vector
vector<string> read_encrypt(string filename);

// Function: read_salts
// Parameters: vector<string>
// Return type: vector<string>
// Description: This function takes in a vector of encrypted words and rips out the salts,
// storing them into a vector
vector<string> read_salts(vector<string> encrypted_words);

// Function: alloc_passwords
// Parameters: vector<string>
// Return type: vector<string>
// Description: This function takes in a vector of dictionary words and stores
// every possible combination inside of a vector
vector<string> alloc_passwords(vector<string> words);

int main(int argc, char** argv){

  MPI_Init(NULL,NULL); // initalizes MPI
  int x,y;
  MPI_Comm_size(MPI_COMM_WORLD,&x); // gets the number of processes
  MPI_Comm_rank(MPI_COMM_WORLD, &y); // gets the individual process number

  string encrypted_words = argv[2]; // will hold the filename for the encrypted words
  string dictionary_filename = argv[1]; // will hold the filename for dict entries

  vector<string> enc_words;
  vector<string> dict_words;
  vector<string> salt_words;
  vector<string> passwords;

  enc_words = read_encrypt(encrypted_words); // holds the encryptions
  dict_words = read_dict(dictionary_filename); // holds the dictionary
  salt_words = read_salts(enc_words); // rips salts from encyrptions

  int n = dict_words.size();
  int m = salt_words.size();
  int o = enc_words.size();
  string testing;
  int counter = 0;

  passwords = alloc_passwords(dict_words); // allocates passwords

  int size = passwords.size(); // total size of vector
  int indexes_to_check = size / (x-1); // amount of indexes each process should check
  int start_index = size; // starting index of each process, modified below

  if(y == 0){
    MPI_Bcast(&indexes_to_check,1, MPI_INT, 0, MPI_COMM_WORLD);
    for(int i = 1; i < x; i++){
      if(i == 1)
        start_index = start_index * 0; // if first process, start at 0
      else{
        start_index = indexes_to_check; // otherwise, start at indexes_to_check
        start_index = start_index * (i - 1); // multiplied by process num - 1
      }
      MPI_Send(&start_index,1,MPI_INT, i , 0, MPI_COMM_WORLD); // send out index
    }
  }

  if(y!= 0){ // if process isnt the root
    MPI_Recv(&start_index, 1, MPI_INT, 0,0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    int count = 0;
    if(y == (x-1)){ // if process is the last one to run
        for(count = start_index; count < size;  count++){
            for(int k = 0; k < m; k++){
                string salt_try;
                string test_enc;
                string current_pass = passwords[count];
                salt_try = salt_words[k];
                test_enc = crypt(current_pass.c_str(), salt_try.c_str());
                for(int l = 0; l < o; l++ ){
                    if(test_enc == enc_words[l]){
                        cout << "Found password: " << passwords[count] << endl;
                    }
                }
            }
         }
    }
    else{
     for(int z = start_index; count < indexes_to_check; z++, count++){
         for(int k = 0; k < m; k++){
             string salt_try;
             string test_enc;
             string current_pass = passwords[z];
             salt_try = salt_words[k];
             test_enc = crypt(current_pass.c_str(), salt_try.c_str());
             for(int l = 0; l < o; l++ ){
                 if(test_enc == enc_words[l]){
                     cout << "Found password: " << passwords[z] << endl;
                 }
             }
         }
      }
    }
  }
MPI_Finalize();
}

vector<string> read_dict (string filename){
  ifstream myfile;
  vector<string> dict_words;
  string placeholder;

  myfile.open(filename.c_str());
  while(getline(myfile, placeholder)){
    dict_words.push_back(placeholder);
  }
  myfile.close();
  return dict_words;
}

vector<string> read_encrypt(string filename){
  ifstream myfile;
  vector<string> encrypt_words;
  string placeholder;

  myfile.open(filename.c_str());
  while(getline(myfile, placeholder)){
    encrypt_words.push_back(placeholder);
  }
  myfile.close();
  return encrypt_words;
}

vector<string> read_salts(vector<string> encrypted_words){
  vector<string> salted_words;
  string placeholder;
  char dollarsign = '$';

  for(int i = 0; i < encrypted_words.size(); i++){
    int last_occurence;
    string encryption;

    encryption = encrypted_words[i];
    last_occurence = encryption.rfind(dollarsign);
    string salt = encryption.substr(0,last_occurence);
    salted_words.push_back(salt);
  }

  return salted_words;
}


vector<string> alloc_passwords(vector<string> dictionary){

  vector<string> passwords;

  for(int i = 0; i < 10; i++){
    for(int y = 0; y < dictionary.size(); y++){
        string num_conversion = to_string(i);
        string dict_word = dictionary[y];

        if(i == 0){  // only push back the word with no nums once each
        passwords.push_back(dict_word);
        }
        passwords.push_back(num_conversion + dict_word);
        passwords.push_back(dict_word + num_conversion);

    }
  }
  // push back the passwords with two numbers
  for(int count = 0; count < 10; count++){
      for(int count1 = 0; count1 < 10; count1++){
          for(int i = 0; i <dictionary.size(); i++){
              string num_conv = to_string(count);
              string num1_conv = to_string(count1);
              string word = dictionary[i];

              passwords.push_back(num_conv + num1_conv + word);
              passwords.push_back(word + num_conv + num1_conv);
          }
      }
  }

  // push back the passwords with three numbers
  for(int count = 0; count < 10; count++){
      for(int count1 = 0; count1 < 10; count1++){
          for(int count2 = 0; count2 < 10; count2++){
              for(int i = 0; i <dictionary.size(); i++){
                  string num_conv = to_string(count);
                  string num1_conv = to_string(count1);
                  string num2_conv = to_string(count2);
                  string word = dictionary[i];

                  passwords.push_back(num_conv + num1_conv + num2_conv + word);
                  passwords.push_back(word + num_conv + num1_conv + num2_conv);
              }
          }
      }
  }
return passwords;
}

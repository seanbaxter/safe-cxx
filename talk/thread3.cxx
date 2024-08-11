#feature on safety
#include "std2.h"
#include <unistd.h>

using namespace std2;

// mutex is sync, so shared_ptr<mutex<string>> is send.
void entry_point(shared_ptr<mutex<string>> data, int thread_id) safe { 
  // Lock the data through the mutex.
  // When lock_guard goes out of scope, the mutex is released.
  auto lock_guard = data->lock();

  // Get a mutable borrow to the string data. When lock_guard goes out of 
  // scope, this becomes a dangling pointer. The borrow checker prevents
  // us from accessing through dangling pointers.
  string^ s = lock_guard^.borrow();

  // Append a fire and print the new shared state.
  s^->append("🔥");

  // Drop the lock before printing the shared state. This makes the borrow
  // `s` a "dangling pointer," in the sense that it depends on an expired
  // lifetime. That will raise a borrowck error on `println(*s)`, which
  // attempts to access shared state outside of the lock.
  // drp lock_guard;
  // drp data;

  // Print the updated shared state.
  println(*s);

  // Sleep 1s before returning.
  unsafe sleep(1);
}

int main() safe {
  auto shared_data = shared_ptr<mutex<string>>::make(string("Hello world - "));

  vector<thread> threads { };

  // Launch all threads.
  const int num_threads = 15;
  for(int i : num_threads)
    threads^.push_back(thread(&entry_point, cpy shared_data, i));
  
  // Join all threads.
  for(thread^ t : ^threads)
    t^->join();
}
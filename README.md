# Password Decoder

## About
<p>Password decoder can help crack the password stored with hashing and salt strategy given the hash code. It will use 
a brute force strategy to help systematically search the password. It uses a thread pool to control multiple threads to 
maximize efficiency. And tasks can be created by other tasks to take advantage of the thread pool structure, 
by appending to the current string all possibilities for one additional character (e.g., the task handling the string 
"123" creates and schedules new tasks for strings "1230", "1231", "1232", etc.)</p>

<p>This project is built for academic purposes for an Introduction to Computer Systems course at UBC.</p>



## How to Use
<p>Step 1 make the file:<br>
make

Step 2 encrypt the target password and the salt:<br>
./encrypt 123456 '$5$12345'

Step 3 feed the hash from step 2 to bruteforce, and specify the number of worker threads in the first argument<br>
./bruteforce 8 '$5$12345$GdwrKB3t6NqmA6t0PDMbXUbpIpr6ODvCyoHWqTVIi2.'

And you can check if it's the same password that is entered in step 2. In real life, you could skip step 2, and only do step 3
with the hash code you get to obtain the password sucessfully.
</p>

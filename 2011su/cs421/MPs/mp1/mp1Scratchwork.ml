(**
  * @file mp1.ml
  * @author Kurt Robert Rudolph
  * @date 2011/06/10 
  * @description 
*)

(* 1.	(1 pt) Declare a variable a with the value 17. It should have type int.*)

let a = 17;;

(*2.	(1 pt) Declare a variable s with a value of "Hi	there". It should have the type of string.*)

let s = "Hi there"

(*3.	(2 pts) Write a function add_a that adds the value of a from Problem 1 to its argument.
# let add_a n = ... ;; val add_a : int -> int = <fun> # add_a 13;; - : int = 30*)

let add_a n = n + a;;


(*4.	(2 pts) Write a function s paired with a times that returns the pair of the string s from Problem 2 paired with the result of multiplying the value a from Problem 1 by the integer input.
# let s_paired_with_a_times b = ... ;; val s_paired_with_a_times : int -> string * int = <fun> # s_paired_with_a_times 12;; - : string * int = ("Hi there", 204)*)

let s_paired_with_a_times b = (s,  a * b);;

(*5.	(3 pts) Write a function abs diff that takes two arguments of type float and returns the absolute value of the difference of the two. Pay careful attention to the type of this problem.
# let abs_diff x y = ... ;; val abs_diff : float -> float -> float = <fun> # abs_diff 15.0 11.5;; - : float = 3.5*)

let abs_diff x y = if (x -. y) > 0.0 then x -. y else -1.0 *. (x -. y);;

(*6.	(3 pts) Write a function greetings that takes a string, which is assumed to be a person’s name, and prints out a greeting as follows: If the name is "Elsa", it prints out the string
"Hey Elsa, cool man!"
(no “newline” at the end), and for any other string, it first prints out "Hello, ", followed by the given name, followed by ". I hope you enjoy CS421.",followedbya“newline”.
# let greetings name = ... ;; val greetings : string -> unit = <fun> # greetings "Angela";; Hello, Angela. I hope you enjoy CS421. - : unit = ()*)

let greetings name = 
	(if name = "Elsa" then 
		(Printf.printf "Hey %s, cool man!" name)
	else 
		(Printf.printf "Hello, %s. I hope you enjoy CS421.\n" name));;

(*7.	(3 pts) Write a function sign that, when given an integer, returns 1 if the integer is positive, 0 if the integer is zero and -1 if the integer is negative.
# let sign n = ... ;; val sign : int -> int = <fun> # sign 4;; - : int = 1*)

let sign n = 
	(if (n > 0) then 
		(1)
	else if (n = 0) then
		(0)
	else 
		(-1));;

(*8.	(5 pts) Write a function pre post compose that takes a pair of functions as a first argument and then takes a second argument and returns the value resulting from applying the first function to the second argument, then applying the second function to the result of the first application, and lastly applying the first function to the result of the second application.
# let pre_post_compose (f,g) x = ... ;; val pre_post_compose : (’a -> ’b) * (’b -> ’a) -> ’a -> ’b = <fun> # pre_post_compose (sign, add_a) 10;; - : int = 1
The correct answer will have the polymorphic type given above, but since we have not discussed polymorphism yet, it will only be tested at the type
pre_post_compose : (int -> int) * (int -> int) -> int -> int
Warning: Any totally correct answer will have the polymorphic type given in the sample output, unless you simply placed a type restriction on one or more subexpressions forcing it to have a less general type than specified.*)

let pre_post_compose (f,g) x = 
	(f( g( f( x))));;


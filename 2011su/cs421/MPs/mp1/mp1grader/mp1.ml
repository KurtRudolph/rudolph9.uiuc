(* CS421 
 * MP1 
 *
 * Please keep in mind that there may be more than one
 * way to solve a problem.  You will want to change how a number of these start.
 *)

open Mp1common

(* Problem 1 *)
let a = 
	17;;

(* Problem 2 *)
let s = 
	"Hi there";;

(* Problem 3 *)
let add_a n =
	n + a;;

(* Problem 4 *)
let s_paired_with_a_times b =
	(s,  a * b);;

(* Problem 5 *)
let abs_diff x y =
	if ((x -. y) > 0.0) then 
		(x -. y)
	else 
		(-1.0 *. (x -. y));;

(* Problem 6 *)
let greetings name =
	(if name = "Elsa" then 
		(print_string "Hey "; print_string name; print_string ", cool man!")
	else 
		(print_string "Hello, "; print_string name; print_string ". I hope you enjoy CS421.\n";));;

(* Problem 7 *)
let sign n =
	(if (n > 0) then 
		(1)
	else if (n = 0) then
		(0)
	else 
		(-1));;

(*Problem 8 *)
let pre_post_compose (f,g) x =
	(f( g( f( x))));;

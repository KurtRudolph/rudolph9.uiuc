(* CS 421 Summer 2011 MP3 *)

open Mp3common

exception Empty_list;;

let rec length list = match list
	with [] -> 0
	|x :: xs -> 1 + length xs;; 

let hd list = match list
	with []	-> raise Empty_list
	| (x :: xs) -> (x);;

let tl list = match list
	with [] -> raise Empty_list
	| (x :: xs) -> (xs)

let rec nth n (x :: xs) = match n  
	with 0 -> x
	| n -> nth (n-1) xs;;

(* Problem 1 *)
let polar_to_cart_helper (r, theta) = 
	(r *. cos theta, r *. sin theta);;

let rec polar_to_cart list = match list 
	with [] -> []
	| (x :: xs) -> polar_to_cart_helper x :: polar_to_cart xs;;

(* Problem 2 *)
let rec determine_app p f g list = match list
	with [] -> []
	| (x :: xs) ->
		 if (p x) then 
			(f x) :: (determine_app p f g xs)
		else 
			(g x) :: (determine_app p f g xs);;
		
(* Problem 3 *)
let sum_sets (x1, y1) (x2, y2) =
	(x1+x2, y1+y2);;

let rec separate_all_from_to m n p = 
	if (m > n) then 
		(0,0)
	else if (p m) then 
		sum_sets (1,0) (separate_all_from_to (m+1) n p)
	else 
	  	sum_sets (0,1) (separate_all_from_to (m+1) n p);;

 
(* Problem 4 *)
let rec all_pred p list = match list
	with [] -> true
	| (x::xs) ->
	if (p x) then 
		all_pred p xs
	else 
		false;; 

(* Problem 5 *)
let rec nearest_helper n n_near list = match list
	with [] -> n_near
	| (x::xs) -> 
		if( abs( n - n_near) > abs( n - x) ) then 
			nearest_helper n x xs
		else 
			nearest_helper n n_near xs;;	

let nearest n list = match list 
	with [] -> 0
	| (x::xs) -> nearest_helper n x xs;;

(* Problem 6 *)
let rec min_max_helper min max f list = match list
	with [] -> (min, max) 
	| (x :: xs) -> 
		if( (f x) < min) then 
			min_max_helper (f x) max f xs
		else if( (f x) > max) then 
			min_max_helper min (f x) f xs
		else 
			min_max_helper min max f xs;;

let min_max f list = match list 
	with [] -> (0,0)
	| (x::xs) -> min_max_helper (f x) (f x) f xs;;

(* Problem 7 *)
let determine_app_base = [];;  (* This may need changing. *)

let determine_app_rec p f g l r =
	if (p l) then (f l)::r else (g l)::r ;;

(* Problem 8 *)
let polar_to_cart_base = [];; (* This may need changing. *)
let polar_to_cart_rec (r, theta) b = (r *. cos theta, r *. sin theta)::b;;

(* Problem 9 *)
let all_pred_base = true;; (* This may need changing. *)
let all_pred_rec p r n = 
	if (p n) then r else false;;

(* Problem 10 *)
let nearest_base lst = hd lst;; 
let nearest_rec n r x = 
	if( abs( n -  r) > abs( n -  x)) then 
		x
	else 
		r;;

(* Problem 11 *)
let app_all_with fs b list =
      List.map (fun f -> (List.map (fun x -> f b x) list)) fs;;

(* Problem 12 *)
let subk n m k = k (m-n) 
let catk a b k = k (a^b)
let absk x k = k (abs x)
let plusk x y k = k (x+.y)
let multk x y k = k (x*.y) 
let is_posk n k = k (n > 0)

(* Problem 13 *)
let abcdk a b c d k = 
	multk (plusk a b (fun j -> j)) (plusk c d (fun l -> l)) k;;

(* Problem 14 *)
let polar_to_cart_helper (r, theta) = 
	 (r *. cos theta, r *. sin theta)

let rec polar_to_cartk list k = match list
	with [] -> k [] 
	| (x::xs) -> polar_to_cartk xs (fun m -> k ((polar_to_cart_helper x)::m ));;

(* Problem 15 *)
let rec all_predk pk list k = match list
	with [] -> k true 
	| (x::xs) -> all_predk pk xs (fun l -> pk x (k));;

(* Problem 16 *)
let nearestk n lst k = raise(Failure "Function not implemented yet.")


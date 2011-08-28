(* 
  CS421 - Summer 2011
  MP2 
*)
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
let reverse_fst_trd (a,b,c) =
	(c,b,a);;

(* Problem 2 *)
let square_dist ((a1, a2),(b1, b2))  =
	((a1-.b1)*.(a1-.b1))+.((a2-.b2)*.(a2-.b2));;

(* Problem 3 *)
let first_two_pairs l1 l2 = 
	if( (length l1 < 2) || (length l2 < 2)) then
		[]
	else
		[(nth 0 l1, nth 0 l2);(nth 1 l1, nth 1 l2)];;

(* Problem 4 *)
let rec sum l = match l
	with [] -> 0
	| (x :: xs) -> x + sum xs;;
	 
(* Problem 5 *)
let prob5helper n = 
	if( (n mod 2) == 0)
		then (n / 2)
	else 
	 	(n * 3 + 1);;
		
let rec collatz n = match n 
	with 1 -> 0
	| n -> 1 + collatz (prob5helper n);; 

(* Problem 6 *)
let unzip [(a1, b1); (a2, b2)] = 
	([a1;a2], [b1;b2]);;

(* Problem 7 *)
let rec dotproduct l1 l2 = match l1 
	with [] -> 0
	| l1 -> match l2
		with [] -> 0
		|l2 -> ((hd l1) * (hd l2)) + (dotproduct (tl l1) (tl l2));;

(* Problem 8 *)
let rec lessThanPartition n list = match list
	with [] -> []
	| (x :: xs) -> 
		if( x < n) then
			x :: lessThanPartition n xs 
		else
			lessThanPartition n xs;;

let rec equalToPartition n list = match list
	with [] -> []
	| (x :: xs) ->
		if( x == n) then
			x :: equalToPartition n xs 
		else
			equalToPartition n xs;;

let rec greaterThanPartition n list = match list
	with [] -> []
	| (x :: xs) -> 
		if( x > n) then
			x :: greaterThanPartition n xs 
		else 
			greaterThanPartition n xs;;

let partition n list = 
	(lessThanPartition n list , equalToPartition n list, greaterThanPartition n list);;

(* Problem 9 *)
let rec remove_all f list = match list
	with [] -> []
	| (x :: xs) -> 
		if( f x) then 
			remove_all f xs
		else 
			x :: remove_all f xs;;

(* Problem 10 *)
let rec mapidxHelper f n list = match list
	with [] -> []
	| (x :: xs) -> (f n x) :: mapidxHelper f (n+1) xs;;

let mapidx f list = 
	mapidxHelper f 0 list;; 

(* Extra Credit *)
(* Problem 11 *)

let rec group list = failwith "not implemented"

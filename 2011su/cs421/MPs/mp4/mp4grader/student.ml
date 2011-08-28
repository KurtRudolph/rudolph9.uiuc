open Mp4common

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

let rec smallest_helper list n = match list
	with [] -> n 
	| (x::xs) -> 
		if ( n > x ) then 
			smallest_helper xs x
		else 
			smallest_helper xs n;;

let smallest list = match list 
	with [] -> raise Empty_list
	| (x :: xs) -> smallest_helper xs x;; 
			

(* Problem 1 *)
let  dept_payroll dept_list = 
	{ id = List.fold_right (fun l -> fun x -> l.id::x) dept_list [] ; salary= List.fold_right (fun l -> fun x -> l.salary +. x) dept_list 0.0};;	

(* Problem 2 *)
let rec sum_smallest_helper list = match list 
		with [] -> 0
		| (x::xs) -> 
			match x with [] -> (sum_smallest_helper xs) 
			| (x_n::xs_n) ->(smallest x) + (sum_smallest_helper xs);;

let rec sum_smaller_checker list = match list 
	with [] -> true
	| (x::xs) -> 
		match x with [] -> false 
		| (x_n ::xs_n) -> sum_smaller_checker xs;;

let sum_smallest list = match list
	with [] -> None
	| (x::xs) -> 
		if( sum_smaller_checker list) then 
			Some (sum_smallest_helper (x::xs))
		else 
			None;;


(* Problem 3 *)
let rec gencompare a b =
	if( a = b) then 
		EQ
	else if( a < b) then 
		LT
	else 
		GT;;

(* Problem 4 *)
let rec insert cmp e list = match list 
	with [] -> (e::[])
	| (x::xs) -> 
		if( (cmp x e) = EQ) then 
			list
		else if( (cmp x e) = LT) then
			x::(insert cmp e xs)
		else 
			e::list;;

(* Problem 5 *)
let compare a1 a2 = 
	match a1 with 
	Pos x1 -> (match a2 with 
		Neg x2 -> if(x1 = x2) then GT else gencompare x1 x2
		| Pos x2 -> gencompare x1 x2)
	| Neg x1 -> (match a2 with 
		Pos x2 -> if(x1 = x2) then LT else gencompare x1 x2
		| Neg x2 -> gencompare x1 x2);; 

(* Problem 6 *)
let rec eliminateImplies p = 
	match p with 
		Atom a -> Atom a
		| Not a -> Not (eliminateImplies a)
		| Or (a, b) -> Or (eliminateImplies a, eliminateImplies b)
		| And (a, b) -> And (eliminateImplies a, eliminateImplies b)
		| Implies (a, b) -> (Or (Not ( eliminateImplies a), eliminateImplies b)) ;;
	

(* Problem 7 *)
let rec eliminateNot_helper p = 
	match p with 
		Atom a ->
			Atom (match a with
				Pos x -> Neg x
				| Neg x -> Pos x)
		| Not a ->  a
		| Or (a, b) -> And (eliminateNot_helper a, eliminateNot_helper b)
		| And (a, b) -> Or (eliminateNot_helper a, eliminateNot_helper b)
		| Implies (a, b) -> Implies (eliminateNot_helper a, eliminateNot_helper b);;
	

let rec eliminateNot p = 
	match p with 
		Atom a -> Atom a
		| Not a -> eliminateNot (eliminateNot_helper a)
		| Or (a, b) -> Or (eliminateNot a, eliminateNot b)
		| And (a, b) -> And (eliminateNot a, eliminateNot b)
		| Implies (a, b) -> Implies (eliminateNot a, eliminateNot b);;

(* Problem 8 *)
let rec dnf p =  
	match p with 
		Atom a -> Atom a
		| Not a -> dnf (eliminateNot p)
		| Or (a, b) -> 
			(match (a, b) with 
				(Or (a_a, a_b), b) -> Or ( Or (dnf a_a, dnf  b), Or (dnf a_b, dnf b))
				| (a, Or (b_a, b_b)) -> Or ( Or (dnf a, dnf b_a), Or (dnf a, dnf b_b))
				| (And (a_a, a_b), b) -> And ( Or (dnf a_a, dnf b), Or (dnf a_b, dnf b))
				| (a, And (b_a, b_b)) -> And ( Or (dnf a, dnf b_a), Or (dnf a, dnf b_b))
				| (a, b) -> Or (dnf a, dnf b))
		| And (a, b) -> 
			(match (a, b) with 
				(Or (a_a, a_b), b) -> Or ( And (dnf a_a, dnf b), And (dnf a_b, dnf b))
				| (a, Or (b_a, b_b)) -> Or ( And (dnf a, dnf b_a), And (dnf a, dnf b_b))
				| (And (a_a, a_b), b) -> And ( And (dnf a_a, dnf b), And (dnf a_b, dnf b))
				| (a, And (b_a, b_b)) -> And ( And (dnf a, dnf b_a), And (dnf a, dnf b_b))
				| (a, b) -> And (dnf a, dnf b))
		| Implies (a, b) -> dnf (eliminateImplies p);;

(* Problem 9 *)
let rec dnf_compact_list p = 
	match p with 
		Atom a -> Conjunction[a]
		| Or (a, b) -> 
			match (a,b) with 
				(Atom a_n, Atom b_n) -> Conjunction[a_n;b_n] 
				| (a,b) -> (dnf_compact_list a; dnf_compact_list b);;
 
let dnf_compact p = 
	match (dnf p) with Or (a,b) ->  Disjunction[
		dnf_compact_list (dnf a); dnf_compact_list (dnf b)];;

(* Problem 10 *)
let rec solve p = match p
	with Disjunction[] -> None
	| Disjunction [Conjunction[a; b]; Conjunction [c]]->(match (a, b) with 
			(Pos a_n, Neg b_n) -> Some [ (a_n, true); (b_n, false)]
			| (Neg a_n, Pos b_n) -> Some [(a_n, false); (b_n, true)]);;
(* Problem 11 *)
let rec satisfies m p = true;;

type 'a payroll_record = {id : 'a; salary : float}

type ord = GT (* greater *) | EQ (* equal *) | LT (* less *)

type 'a atom = Pos of 'a | Neg of 'a

type 'a prop = 
  | Atom of 'a atom
  | Not of 'a prop
  | Or of 'a prop * 'a prop 
  | And of 'a prop * 'a prop
  | Implies of 'a prop * 'a prop

type boolVar = A | B | C | D | E

type 'a conj = Conjunction of 'a atom list
type 'a disj = Disjunction of 'a conj list

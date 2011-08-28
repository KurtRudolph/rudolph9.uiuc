(*
 * File: mp5-skeleton.ml
 *)
open Mp5common

let rec gather_exp_ty_constraints judgment =
    let {gamma_exp = gamma; exp = exp; result = tau} = judgment in
    match exp
    with ConstExp c ->
      let tau' = const_signature c in 
         Some ({antecedents = []; conclusion = ExpJudgment judgment}, [(tau, tau')])
(* You add clauses here *)

and gather_dec_ty_constraints gamma dec =

    match dec with Val (v,e) ->
       let tau = fresh() in
       let env_inc = make_env v tau in
       option_map
       (fun (e_pf, c) ->
        ({antecedents=[e_pf];
         conclusion =
          DecJudgment {gamma_dec = gamma; dec = dec; result_env = env_inc}},
        env_inc,
        c))
        (gather_exp_ty_constraints {gamma_exp = gamma; exp = e; result = tau})


(* You add clauses here *)




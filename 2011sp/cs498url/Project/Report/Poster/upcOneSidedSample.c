shared int64_t *pred2= (shared int64_t*) 
  upc_all_alloc( nlocalverts * sizeof( int64_t));
shared int64)
shared int64_t *local_vertices= (shared int64_t*)
  upc_all_alloc(nlocalverts * sizeof(int64_t));

upc_all_reduceI( (shared int64_t*) pred2,  
  (shared int64_t*) (local_vertices + v_local), 
  (upc_op_t) UPC_MIN, 
  (int64_t) (nlocalverts * THREADS),
  (int64_t) nlocalverts, NULL, 
  (upc_flag_t) sync_mode);

upc_free(pred2); upc_free(local_vertices);

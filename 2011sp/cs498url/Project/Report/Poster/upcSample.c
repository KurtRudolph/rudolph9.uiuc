#define N 10000
shared int *vectorA; 
shared int *vectorB; 
shared int *vectorC; 
// C = A \dot B
void dotProduct() {
 int i;
 upc_forall( i= 0; 
       i< N; i++, i) {
  vectorC[i]= vectorA[i]*vectorB[i];
 }
 upc_all_reduceI( 
   (shared int*) vectorC, 
   (shared int*) vectorC,
   (upc_op_t) UPC_SUM,
   THREADS, 1, 
   (upc_flag_t) sync_mode); 
}



    

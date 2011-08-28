//Open window
MPI_Win_create(pred, nlocalverts * sizeof(int64_t), 
  sizeof(int64_t), MPI_INFO_NULL, 
  MPI_COMM_WORLD, &pred_win);
//close window
MPI_Win_free(&pred_win);

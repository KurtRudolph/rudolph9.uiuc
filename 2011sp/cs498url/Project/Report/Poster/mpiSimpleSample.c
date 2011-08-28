MPI_Irecv( recvbuf, coalescSize * 2, INT64_T_MPI_TYPE,
  MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &recvreq);

MPI_Isend( &outgoing[ dest * coalescSize * 2], 0, 
  INT64_T_MPI_TYPE, dest, 0, 
  MPI_COMM_WORLD, &outgoing_reqs[dest]);

/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/* 
 *
 *   Copyright (C) 2004 University of Chicago. 
 *   See COPYRIGHT notice in top-level directory.
 */

#include "adio.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef AGGREGATION_PROFILE
#include "mpe.h"
#endif

void ADIOI_GEN_WriteContig(ADIO_File fd, const void *buf, int count,
			   MPI_Datatype datatype, int file_ptr_type,
			   ADIO_Offset offset, ADIO_Status *status,
			   int *error_code)
{
    int err = -1, datatype_size;
    ADIO_Offset len;
    static char myname[] = "ADIOI_GEN_WRITECONTIG";

#ifdef AGGREGATION_PROFILE
    MPE_Log_event (5036, 0, NULL);
#endif

    MPI_Type_size(datatype, &datatype_size);
    len = (ADIO_Offset)datatype_size * (ADIO_Offset)count;
    ADIOI_Assert(len == (unsigned int) len); /* read takes an unsigned int parm */

    if (file_ptr_type == ADIO_INDIVIDUAL) {
	offset = fd->fp_ind;
    }

    if (fd->fp_sys_posn != offset) {
#ifdef ADIOI_MPE_LOGGING
        MPE_Log_event( ADIOI_MPE_lseek_a, 0, NULL );
#endif
	err = lseek(fd->fd_sys, offset, SEEK_SET);
#ifdef ADIOI_MPE_LOGGING
        MPE_Log_event( ADIOI_MPE_lseek_b, 0, NULL );
#endif
	/* --BEGIN ERROR HANDLING-- */
	if (err == -1) {
	    *error_code = MPIO_Err_create_code(MPI_SUCCESS,
					       MPIR_ERR_RECOVERABLE,
					       myname, __LINE__,
					       MPI_ERR_IO, "**io",
					       "**io %s", strerror(errno));
	    fd->fp_sys_posn = -1;
	    return;
	}
	/* --END ERROR HANDLING-- */
    }
    
#ifdef ADIOI_MPE_LOGGING
    MPE_Log_event( ADIOI_MPE_write_a, 0, NULL );
#endif
    err = write(fd->fd_sys, buf, (unsigned int)len);
#ifdef ADIOI_MPE_LOGGING
    MPE_Log_event( ADIOI_MPE_write_b, 0, NULL );
#endif
    /* --BEGIN ERROR HANDLING-- */
    if (err == -1) {
	*error_code = MPIO_Err_create_code(MPI_SUCCESS,
					   MPIR_ERR_RECOVERABLE,
					   myname, __LINE__,
					   MPI_ERR_IO, "**io",
					   "**io %s", strerror(errno));
	fd->fp_sys_posn = -1;
	return;
    }
    /* --END ERROR HANDLING-- */

    fd->fp_sys_posn = offset + err;

    if (file_ptr_type == ADIO_INDIVIDUAL) {
	fd->fp_ind += err; 
    }

#ifdef HAVE_STATUS_SET_BYTES
    if (err != -1 && status) MPIR_Status_set_bytes(status, datatype, err);
#endif

    *error_code = MPI_SUCCESS;
#ifdef AGGREGATION_PROFILE
    MPE_Log_event (5037, 0, NULL);
#endif
}

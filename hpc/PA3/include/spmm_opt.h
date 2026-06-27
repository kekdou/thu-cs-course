#ifndef SpMM_OPT_H
#define SpMM_OPT_H
#include "spmm_base.h"

class SpMMOpt : public SpMM {
public:
    SpMMOpt(int *dev_out_ptr, int *dev_out_idx, int out_num_v, int out_num_e, int out_feat_in) : SpMM(dev_out_ptr, dev_out_idx, out_num_v, out_num_e, out_feat_in) {}
    SpMMOpt(CSR *g, int out_feat_in) : SpMM(g, out_feat_in) {}
    ~SpMMOpt() {
        if (first_start) checkCudaErrors(cudaFree(first_start));
        if (first_len) checkCudaErrors(cudaFree(first_len));
        if (first_row) checkCudaErrors(cudaFree(first_row));
        if (rest_start) checkCudaErrors(cudaFree(rest_start));
        if (rest_len) checkCudaErrors(cudaFree(rest_len));
        if (rest_row) checkCudaErrors(cudaFree(rest_row));
    }
    virtual void preprocess(float *vin, float *vout);
    virtual void run(float *vin, float *vout);

private:
    int first_count = 0;
    int rest_count = 0;
    int total_nnz = 0;
    int *first_start = NULL, *first_len = NULL, *first_row = NULL;
    int *rest_start = NULL, *rest_len = NULL, *rest_row = NULL;
};
#endif

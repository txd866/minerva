#include "basic.h"
#include "common/nvector.h"

#include <cmath>
#include <glog/logging.h>

using namespace std;

namespace minerva {
namespace basic {

void Arithmetic(DataList& inputs, DataList& outputs, ArithmeticClosure& closure) {
  CHECK_EQ(inputs.size(), 2) << "(arithmetic) #inputs is wrong!";
  CHECK_EQ(outputs.size(), 1) << "(arithmetic) #outputs is wrong!";
  float* left_data = inputs[0].GetCpuData();
  float* right_data = inputs[1].GetCpuData();
  float* res_data = outputs[0].GetCpuData();
  int length = outputs[0].Size().Prod();
  switch(closure.type) {
    case ADD:
      for (int i = 0; i < length; ++i) {
        res_data[i] = left_data[i] + right_data[i];
      }
      break;
    case SUB:
      for (int i = 0; i < length; ++i) {
        res_data[i] = left_data[i] - right_data[i];
      }
      break;
    case MULT:
      for (int i = 0; i < length; ++i) {
        res_data[i] = left_data[i] * right_data[i];
      }
      break;
    case DIV:
      for (int i = 0; i < length; ++i) {
        res_data[i] = left_data[i] / right_data[i];
      }
      break;
  }
}

void ArithmeticConst(DataList& inputs, DataList& outputs, ArithmeticConstClosure& closure) {
  CHECK_EQ(inputs.size(), 1) << "(arithmetic const) #inputs is wrong!";
  CHECK_EQ(outputs.size(), 1) << "(arithmetic const) #outputs is wrong!";
  float val = closure.val;
  float* in_data = inputs[0].GetCpuData();
  float* res_data = outputs[0].GetCpuData();
  int length = outputs[0].Size().Prod();
  switch(closure.type) {
    case ADD:
      if(closure.side == 0) {// const on left
        for (int i = 0; i < length; ++i) {
          res_data[i] = in_data[i] + val;
        }
      } else {// const on right
        for (int i = 0; i < length; ++i) {
          res_data[i] = val + in_data[i];
        }
      }
      break;
    case SUB:
      if(closure.side == 0) {// const on left
        for (int i = 0; i < length; ++i) {
          res_data[i] = in_data[i] - val;
        }
      } else {// const on right
        for (int i = 0; i < length; ++i) {
          res_data[i] = val - in_data[i];
        }
      }
      break;
    case MULT:
      if(closure.side == 0) {// const on left
        for (int i = 0; i < length; ++i) {
          res_data[i] = in_data[i] * val;
        }
      } else {// const on right
        for (int i = 0; i < length; ++i) {
          res_data[i] = val * in_data[i];
        }
      }
      break;
    case DIV:
      if(closure.side == 0) {// const on left
        for (int i = 0; i < length; ++i) {
          res_data[i] = in_data[i] / val;
        }
      } else {// const on right
        for (int i = 0; i < length; ++i) {
          res_data[i] = val / in_data[i];
        }
      }
      break;
  }
}

void Elewise(DataList& inputs, DataList& outputs, ElewiseClosure& closure) {
  CHECK_EQ(inputs.size(), 1) << "(elewise) #inputs is wrong!";
  CHECK_EQ(outputs.size(), 1) << "(elewise) #outputs is wrong!";
  float* in_data = inputs[0].GetCpuData();
  float* res_data = outputs[0].GetCpuData();
  int length = outputs[0].Size().Prod();
  switch(closure.type) {
    case EXP:
      for (int i = 0; i < length; ++i) {
        res_data[i] = exp(in_data[i]);
      }
      break;
    case LN:
      for (int i = 0; i < length; ++i) {
        res_data[i] = log(in_data[i]);
      }
      break;
    case SIGMOID:
      for (int i = 0; i < length; ++i) {
        res_data[i] = 1 / (1 + exp(-in_data[i]));
      }
      break;
    case NEGATIVE:
      for (int i = 0; i < length; ++i) {
        res_data[i] = -in_data[i];
      }
      break;
  }
}

void MatMult(DataList& inputs, DataList& outputs, MatMultClosure& closure) {
  CHECK_EQ(inputs.size(), 2) << "(matmult) #inputs is wrong!";
  CHECK_EQ(outputs.size(), 1) << "(matmult) #outputs is wrong!";
  float* left_data = inputs[0].GetCpuData();
  float* right_data = inputs[1].GetCpuData();
  float* res_data = outputs[0].GetCpuData();
  int m = outputs[0].Size()[0];
  int n = outputs[0].Size()[1];
  int o = inputs[0].Size()[1];
  // ATTENTION: the data is column major !!
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < n; ++j) {
      res_data[i + j * m] = 0;
      for (int k = 0; k < o; ++k) {
        res_data[i + j * m] += left_data[i + k * m] * right_data[k + j * o];
      }
    }
  }
}

void Transpose(DataList& inputs, DataList& outputs, TransposeClosure& closure) {
  CHECK_EQ(inputs.size(), 1) << "(transpose) #inputs is wrong!";
  CHECK_EQ(outputs.size(), 1) << "(transpose) #outputs is wrong!";
  float* in_data = inputs[0].GetCpuData();
  float* res_data = outputs[0].GetCpuData();
  int m = outputs[0].Size()[0];
  int n = outputs[0].Size()[1];
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < n; ++j) {
      res_data[i + j * m] = in_data[j + i * n];
    }
  }
}

void Reduction(DataList& inputs, DataList& outputs, ReductionClosure& closure) {
  CHECK_EQ(inputs.size(), 1) << "(reduction) #inputs is wrong!";
  CHECK_EQ(outputs.size(), 1) << "(reduction) #outputs is wrong!";
  float* in_data = inputs[0].GetCpuData();
  float* res_data = outputs[0].GetCpuData();
  auto in_max = inputs[0].Size();
  auto in_range = ScaleRange::MakeRangeFromOrigin(in_max);
  auto res_max = outputs[0].Size();
  auto res_range = ScaleRange::MakeRangeFromOrigin(res_max);
  auto accumulator = Scale::Origin(in_max.NumDims());
  do {
    auto cur = accumulator;
    float tmp = in_data[in_range.Flatten(cur)];
    while (cur.IncrDimensions(in_max, closure.dims_to_reduce)) {
      float tmp2 = in_data[in_range.Flatten(cur)];
      // TODO Moving switch out of loop to optimize
      switch (closure.type) {
        case SUM:
          tmp += tmp2;
          break;
        case MAX:
          if (tmp < tmp2) {
            tmp = tmp2;
          }
          break;
      }
    }
    res_data[res_range.Flatten(accumulator)] = tmp;
  } while (accumulator.IncrWithDimensionsFixed(res_max, closure.dims_to_reduce));
}

void Randn(DataList& output, RandnClosure& closure) {
  CHECK_EQ(output.size(), 1) << "wrong number of randn output";
  int length = output[0].Size().Prod();
  float* data = output[0].GetCpuData();
  default_random_engine generator;
  normal_distribution<float> distribution(closure.mu, closure.var); // TODO only float for now
  for (int i = 0; i < length; ++i) {
    data[i] = distribution(generator);
  }
}

void Fill(DataList& output, FillClosure& closure) {
  CHECK_EQ(output.size(), 1) << "wrong number of fill constant output";
  int length = output[0].Size().Prod();
  float* data = output[0].GetCpuData();
  for (int i = 0; i < length; ++i) {
    data[i] = closure.val;
  }
}

void Assemble(DataList& inputs, DataList& outputs, AssembleClosure& closure) {
  CHECK_EQ(outputs.size(), 1) << "wrong number of assemble output";
  size_t numdims = outputs[0].Size().NumDims();
  float* dest = outputs[0].GetCpuData();
  Scale srcstart = Scale::Origin(numdims);
  for (auto& i: inputs) {
    NCopy(i.GetCpuData(), i.Size(), srcstart,
        dest, outputs[0].Size(), i.Offset(),
        i.Size());
  }
}

void Split(DataList& inputs, DataList& outputs, SplitClosure& closure) {
  CHECK_EQ(inputs.size(), 1) << "wrong number of split input";
  DataShard& inds = inputs[0];
  size_t numdims = inds.Size().NumDims();
  float* src = inds.GetCpuData();
  Scale dststart = Scale::Origin(numdims);
  for(DataShard& outds : outputs) {
    NCopy(src, inds.Size(), outds.Offset(),
        outds.GetCpuData(), outds.Size(), dststart,
        outds.Size());
  }
}

void NCopy(float* src, const Scale& srcsize, const Scale& srcstart,
    float* dst, const Scale& dstsize, const Scale& dststart,
    const Scale& copysize) {
  size_t numdims = srcsize.NumDims();
  CHECK_EQ(srcstart.NumDims(), numdims) << "copy error: wrong #dims";
  CHECK_EQ(copysize.NumDims(), numdims) << "copy error: wrong #dims";
  CHECK_EQ(dstsize.NumDims(), numdims) << "copy error: wrong #dims";
  CHECK_EQ(dststart.NumDims(), numdims) << "copy error: wrong #dims";
  Scale srcend = srcstart + copysize;
  Scale dstend = dststart + copysize;
  CHECK_LE(dstend, dstsize) << "copy error: not enough dest space";
  Scale percopysize = Scale::Constant(numdims, 1);
  for(size_t i = 0; i < numdims; ++i) {
    percopysize[i] = copysize[i];
    if(!(srcstart[i] == 0 && srcend[i] == srcsize[i]
          && dststart[i] == 0 && dstend[i] == dstsize[i])) {
      // remainings are non-contigous parts
      break;
    }
  }
  Scale copystart = Scale::Origin(numdims);
  ScaleRange srcrange = ScaleRange::MakeRangeFromOrigin(srcsize);
  ScaleRange dstrange = ScaleRange::MakeRangeFromOrigin(dstsize);
  int copytimes = 0;
  int percopylen = percopysize.Prod();
  do {
    size_t srcoff = srcrange.Flatten(srcstart + copystart);
    size_t dstoff = dstrange.Flatten(dststart + copystart);
    // do memcopy
    memcpy(dst + dstoff, src + srcoff, percopylen * sizeof(float));
    ++copytimes;
    // incr copy_start
    copystart = copystart + percopysize - 1; // similar to "end = start + len - 1"
  } while(Scale::IncrOne(copystart, copysize));
  VLOG(1) << "Copy times in NCopy:" << copytimes;
}

} // end of namespace basic
} // end of namespace minerva

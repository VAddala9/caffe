#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <vector>


#include "caffe/layer.hpp"
#include "caffe/layers/simple_crop_layer.hpp"
#include "caffe/net.hpp"


namespace caffe {

template <typename Dtype>
void SimpleCropLayer<Dtype>::LayerSetUp(const vector<Blob<Dtype>*>& bottom,
    const vector<Blob<Dtype>*>& top) {
  // LayerSetup() handles the number of dimensions; Reshape() handles the sizes.
  // bottom[0] supplies the data
  // bottom[1] supplies the size
  const BlobShape& crop_shape = this->layer_param_.simple_crop_param().shape();


  const int num_crop = crop_shape.dim_size();
  CHECK_EQ(num_crop, 2) << "too many axis for crop :" 
      << num_crop;
  top_shape_ = vector<int>(bottom[0]->num_axes());
  int i;
  for (i = 0; i < bottom[0]->num_axes()-2; ++i) {
	  top_shape_[i] = bottom[0]->shape(i);
  }
  top_shape_[i] = crop_shape.dim(0);
  crop_h_ = round((bottom[0]->shape(i)-top_shape_[i])/2.0);
  i++;
  top_shape_[i] = crop_shape.dim(1);
  crop_w_ = round((bottom[0]->shape(i)-top_shape_[i])/2.0);
}

template <typename Dtype>
void SimpleCropLayer<Dtype>::Reshape(const vector<Blob<Dtype>*>& bottom,
    const vector<Blob<Dtype>*>& top) {
  top[0]->Reshape(top_shape_);
}


template <typename Dtype>
void SimpleCropLayer<Dtype>::Forward_cpu(const vector<Blob<Dtype>*>& bottom,
    const vector<Blob<Dtype>*>& top) {
  const Dtype* bottom_data = bottom[0]->cpu_data();
  Dtype* top_data = top[0]->mutable_cpu_data();
  for (int n = 0; n < top[0]->num(); ++n) {
    for (int c = 0; c < top[0]->channels(); ++c) {
      for (int h = 0; h < top[0]->height(); ++h) {
        caffe_copy(top[0]->width(),
            bottom_data + bottom[0]->offset(n, c, crop_h_ + h, crop_w_),
            top_data + top[0]->offset(n, c, h));
      }
    }
  }

}

template <typename Dtype>
void SimpleCropLayer<Dtype>::Backward_cpu(const vector<Blob<Dtype>*>& top,
    const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom) {
  const Dtype* top_diff = top[0]->cpu_diff();
  Dtype* bottom_diff = bottom[0]->mutable_cpu_diff();

  if (propagate_down[0]) {
    caffe_set(bottom[0]->count(), static_cast<Dtype>(0), bottom_diff);
    for (int n = 0; n < top[0]->num(); ++n) {
      for (int c = 0; c < top[0]->channels(); ++c) {
        for (int h = 0; h < top[0]->height(); ++h) {
          caffe_copy(top[0]->width(),
              top_diff + top[0]->offset(n, c, h),
              bottom_diff + bottom[0]->offset(n, c, crop_h_ + h, crop_w_));
        }
      }
    }
  }
}


INSTANTIATE_CLASS(SimpleCropLayer);
REGISTER_LAYER_CLASS(SimpleCrop);

}  // namespace caffe

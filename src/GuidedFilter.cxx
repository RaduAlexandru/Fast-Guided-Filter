#include "baseline/GuidedFilter.h"

// GuidedFilter::GuidedFilter(){
//   std::cout << "gf constructor" << '\n';
// }
static cv::Mat boxfilter(const cv::Mat &I, int r)
{
    cv::Mat result;
    cv::blur(I, result, cv::Size(r, r));
    return result;
}

static cv::Mat convertTo(const cv::Mat &mat, int depth)
{
    if (mat.depth() == depth)
        return mat;

    cv::Mat result;
    mat.convertTo(result, depth);
    return result;
}

class GuidedFilterImpl
{
public:
    virtual ~GuidedFilterImpl() {}

    cv::Mat filter(const cv::Mat &p, int scale, int depth);


protected:
    int Idepth;

private:
    virtual cv::Mat filterSingleChannel(const cv::Mat &p) const = 0;
    virtual cv::Mat filterSingleChannelFast(const cv::Mat &p) const = 0;
};

class GuidedFilterMono : public GuidedFilterImpl
{
public:
    GuidedFilterMono(const cv::Mat &I, int r, double eps, int scale);

private:
    virtual cv::Mat filterSingleChannel(const cv::Mat &p) const;
    virtual cv::Mat filterSingleChannelFast(const cv::Mat &p) const;

private:
    int r;
    double eps;
    cv::Mat I, mean_I, var_I;
    //Stuff necesarry in the case of subsampling
    cv::Mat I_sub, p_sub;
    int r_sub;
    int scale;
    cv::Size size_orig, size_subsampled;


    //Other stuff that are nice to have as class members
    cv::Mat mean_II;

};

class GuidedFilterColor : public GuidedFilterImpl
{
public:
    GuidedFilterColor(const cv::Mat &I, int r, double eps, int scale);

private:
    virtual cv::Mat filterSingleChannel(const cv::Mat &p) const;
    virtual cv::Mat filterSingleChannelFast(const cv::Mat &p) const;

private:
    std::vector<cv::Mat> Ichannels;
    int r;
    double eps;
    cv::Mat mean_I_r, mean_I_g, mean_I_b;
    cv::Mat invrr, invrg, invrb, invgg, invgb, invbb;



    //Stuff necesarry in the case of subsampling
    cv::Mat I_sub, p_sub;
    int r_sub;
    int scale;


    //Other stuff that are nice to have as class members
    cv::Mat mean_II;
};


cv::Mat GuidedFilterImpl::filter(const cv::Mat &p, int scale  ,int depth)
{
    cv::Mat p2 = convertTo(p, Idepth);

    cv::Mat result;


    if (scale==1){
      if (p.channels() == 1)
      {
          result = filterSingleChannel(p2);
      }
      else
      {
          std::vector<cv::Mat> pc;
          cv::split(p2, pc);

          for (std::size_t i = 0; i < pc.size(); ++i)
              pc[i] = filterSingleChannel(pc[i]);

          cv::merge(pc, result);
      }
    }else{
      if (p.channels() == 1)
      {
          result = filterSingleChannelFast(p2);
      }else{
        std::vector<cv::Mat> pc;
        cv::split(p2, pc);

        for (std::size_t i = 0; i < pc.size(); ++i)
            pc[i] = filterSingleChannelFast(pc[i]);

        cv::merge(pc, result);
      }


    }


    return convertTo(result, depth == -1 ? p.depth() : depth);
}

GuidedFilterMono::GuidedFilterMono(const cv::Mat &origI, int r, double eps, int scale) : r(r), eps(eps), scale(scale)
{
    if (origI.depth() == CV_32F || origI.depth() == CV_64F)
        I = origI.clone();
    else
        I = convertTo(origI, CV_32F);

    Idepth = I.depth();


    if (scale!=1){
      size_orig= I.size();
      size_subsampled=cv::Size(I.cols/scale, I.rows/scale);

      r_sub=r/scale;
      resize(I,I_sub,size_subsampled, 0.0 ,0.0 ,  cv::INTER_NEAREST);
      mean_I = boxfilter(I_sub, r_sub);
      mean_II = boxfilter(I_sub.mul(I_sub), r_sub);
    }else{
      mean_I = boxfilter(I, r);
      mean_II = boxfilter(I.mul(I), r);
    }
    var_I = mean_II - mean_I.mul(mean_I);
}

cv::Mat GuidedFilterMono::filterSingleChannel(const cv::Mat &p) const
{
    cv::Mat mean_p = boxfilter(p, r);
    cv::Mat mean_Ip = boxfilter(I.mul(p), r);
    cv::Mat cov_Ip = mean_Ip - mean_I.mul(mean_p); // this is the covariance of (I, p) in each local patch.

    cv::Mat a = cov_Ip / (var_I + eps); // Eqn. (5) in the paper;
    cv::Mat b = mean_p - a.mul(mean_I); // Eqn. (6) in the paper;

    cv::Mat mean_a = boxfilter(a, r);
    cv::Mat mean_b = boxfilter(b, r);

    return mean_a.mul(I) + mean_b;
}


cv::Mat GuidedFilterMono::filterSingleChannelFast(const cv::Mat &p) const {


    // std::cout << "scale is " << scale << '\n';
    // std::cout << "r_sub is " << r_sub << '\n';
    cv::Mat p_sub;
    cv::resize(p,p_sub,size_subsampled, 0.0 ,0.0 ,  cv::INTER_NEAREST);
    cv::Mat mean_p = boxfilter(p_sub, r_sub);
    cv::Mat mean_Ip = boxfilter(I_sub.mul(p_sub), r_sub);
    cv::Mat cov_Ip = mean_Ip - mean_I.mul(mean_p); // this is the covariance of (I, p) in each local patch.

    cv::Mat a = cov_Ip / (var_I + eps); // Eqn. (5) in the paper;
    cv::Mat b = mean_p - a.mul(mean_I); // Eqn. (6) in the paper;


    cv::Mat mean_a = boxfilter(a, r_sub);
    cv::Mat mean_b = boxfilter(b, r_sub);

    //upsample
    cv::resize(mean_a, mean_a, size_orig);
    cv::resize(mean_b, mean_b, size_orig);



    return mean_a.mul(I) + mean_b;
}








GuidedFilterColor::GuidedFilterColor(const cv::Mat &origI, int r, double eps, int scale) : r(r), eps(eps), scale(scale)
{
    cv::Mat I;
    if (origI.depth() == CV_32F || origI.depth() == CV_64F)
        I = origI.clone();
    else
        I = convertTo(origI, CV_32F);

    Idepth = I.depth();

    cv::split(I, Ichannels);

    mean_I_r = boxfilter(Ichannels[0], r);
    mean_I_g = boxfilter(Ichannels[1], r);
    mean_I_b = boxfilter(Ichannels[2], r);

    // variance of I in each local patch: the matrix Sigma in Eqn (14).
    // Note the variance in each local patch is a 3x3 symmetric matrix:
    //           rr, rg, rb
    //   Sigma = rg, gg, gb
    //           rb, gb, bb
    cv::Mat var_I_rr = boxfilter(Ichannels[0].mul(Ichannels[0]), r) - mean_I_r.mul(mean_I_r) + eps;
    cv::Mat var_I_rg = boxfilter(Ichannels[0].mul(Ichannels[1]), r) - mean_I_r.mul(mean_I_g);
    cv::Mat var_I_rb = boxfilter(Ichannels[0].mul(Ichannels[2]), r) - mean_I_r.mul(mean_I_b);
    cv::Mat var_I_gg = boxfilter(Ichannels[1].mul(Ichannels[1]), r) - mean_I_g.mul(mean_I_g) + eps;
    cv::Mat var_I_gb = boxfilter(Ichannels[1].mul(Ichannels[2]), r) - mean_I_g.mul(mean_I_b);
    cv::Mat var_I_bb = boxfilter(Ichannels[2].mul(Ichannels[2]), r) - mean_I_b.mul(mean_I_b) + eps;

    // Inverse of Sigma + eps * I
    invrr = var_I_gg.mul(var_I_bb) - var_I_gb.mul(var_I_gb);
    invrg = var_I_gb.mul(var_I_rb) - var_I_rg.mul(var_I_bb);
    invrb = var_I_rg.mul(var_I_gb) - var_I_gg.mul(var_I_rb);
    invgg = var_I_rr.mul(var_I_bb) - var_I_rb.mul(var_I_rb);
    invgb = var_I_rb.mul(var_I_rg) - var_I_rr.mul(var_I_gb);
    invbb = var_I_rr.mul(var_I_gg) - var_I_rg.mul(var_I_rg);

    cv::Mat covDet = invrr.mul(var_I_rr) + invrg.mul(var_I_rg) + invrb.mul(var_I_rb);

    invrr /= covDet;
    invrg /= covDet;
    invrb /= covDet;
    invgg /= covDet;
    invgb /= covDet;
    invbb /= covDet;
}

cv::Mat GuidedFilterColor::filterSingleChannel(const cv::Mat &p) const
{
    cv::Mat mean_p = boxfilter(p, r);

    cv::Mat mean_Ip_r = boxfilter(Ichannels[0].mul(p), r);
    cv::Mat mean_Ip_g = boxfilter(Ichannels[1].mul(p), r);
    cv::Mat mean_Ip_b = boxfilter(Ichannels[2].mul(p), r);

    // covariance of (I, p) in each local patch.
    cv::Mat cov_Ip_r = mean_Ip_r - mean_I_r.mul(mean_p);
    cv::Mat cov_Ip_g = mean_Ip_g - mean_I_g.mul(mean_p);
    cv::Mat cov_Ip_b = mean_Ip_b - mean_I_b.mul(mean_p);

    cv::Mat a_r = invrr.mul(cov_Ip_r) + invrg.mul(cov_Ip_g) + invrb.mul(cov_Ip_b);
    cv::Mat a_g = invrg.mul(cov_Ip_r) + invgg.mul(cov_Ip_g) + invgb.mul(cov_Ip_b);
    cv::Mat a_b = invrb.mul(cov_Ip_r) + invgb.mul(cov_Ip_g) + invbb.mul(cov_Ip_b);

    cv::Mat b = mean_p - a_r.mul(mean_I_r) - a_g.mul(mean_I_g) - a_b.mul(mean_I_b); // Eqn. (15) in the paper;

    return (boxfilter(a_r, r).mul(Ichannels[0])
          + boxfilter(a_g, r).mul(Ichannels[1])
          + boxfilter(a_b, r).mul(Ichannels[2])
          + boxfilter(b, r));  // Eqn. (16) in the paper;
}


cv::Mat GuidedFilterColor::filterSingleChannelFast(const cv::Mat &p) const
{
  std::cout << "NOT YET IMPLEMENTED" << '\n';
}


GuidedFilter::GuidedFilter(const cv::Mat &I, int r, double eps, int scale):
  scale(scale){
    CV_Assert(I.channels() == 1 || I.channels() == 3);

    if (I.channels() == 1)
        impl_ = new GuidedFilterMono(I, 2 * r + 1, eps,  scale);
    else
        impl_ = new GuidedFilterColor(I, 2 * r + 1, eps, scale);
}

GuidedFilter::~GuidedFilter()
{
    delete impl_;
}

cv::Mat GuidedFilter::filter(const cv::Mat &p, int depth) const
{
    return impl_->filter(p, scale, depth);
}

cv::Mat guidedFilter(const cv::Mat &I, const cv::Mat &p, int r, double eps, int scale, int depth)
{
    return GuidedFilter(I, r, eps, scale).filter(p, depth);
}

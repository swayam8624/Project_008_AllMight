#pragma once
// P22: paraxial, ideal thin lens in millimetres; object z is positive
// on the object side, sensor distance positive on the opposite side.
#include <cmath>
#include <stdexcept>
#include <optional>
#include <numbers>
namespace book03 {
struct OpticalPoint {double x{},y{};};
struct FocusInterval {double near_mm{};std::optional<double> far_mm;};
struct ThinLens {
    double focal_mm{50},f_number{8},focus_mm{3000};
    void validate()const{
        if(!std::isfinite(focal_mm)||focal_mm<1||focal_mm>1000||
           !std::isfinite(f_number)||f_number<.5||f_number>128||
           !std::isfinite(focus_mm)||focus_mm-focal_mm<.01||focus_mm>1e9)
            throw std::invalid_argument("lens outside teaching domain");
    }
    double image_distance(double z)const{
        validate();if(!std::isfinite(z)||z-focal_mm<.01||z>1e9)
            throw std::invalid_argument("real-image object distance required");
        return focal_mm+focal_mm*focal_mm/(z-focal_mm);
    }
    double aperture_mm()const{validate();return focal_mm/f_number;}
    double sensor_mm()const{return image_distance(focus_mm);}
    double blur_mm(double z)const{
        (void)image_distance(z);
        return focal_mm*focal_mm/f_number/(focus_mm-focal_mm)*std::abs(1-focus_mm/z);
    }
    double infinity_blur_mm()const{validate();return focal_mm*focal_mm/f_number/(focus_mm-focal_mm);}
    FocusInterval acceptable(double c)const{
        validate();if(!std::isfinite(c)||c<1e-6||c>10)throw std::invalid_argument("blur tolerance");
        const double k=c/infinity_blur_mm();
        return {focus_mm/(1+k),k>=1?std::nullopt:std::optional<double>{focus_mm/(1-k)}};
    }
    double hyperfocal_mm(double c)const{
        validate();if(!std::isfinite(c)||c<1e-6||c>10)throw std::invalid_argument("blur tolerance");
        return focal_mm+focal_mm*focal_mm/(f_number*c);
    }
    // Intersect an aperture ray with the fixed sensor plane. The lens
    // changes incoming slope (a-X)/z by -a/f, independently per axis.
    OpticalPoint sensor_hit(OpticalPoint object,double z,OpticalPoint pupil)const{
        (void)image_distance(z);const double r=aperture_mm()/2,s=sensor_mm();
        if(!std::isfinite(object.x)||!std::isfinite(object.y)||
           std::abs(object.x)>1e6||std::abs(object.y)>1e6||
           !std::isfinite(pupil.x)||!std::isfinite(pupil.y)||std::hypot(pupil.x,pupil.y)>r*(1+1e-12))
            throw std::invalid_argument("object or pupil outside domain");
        auto hit=[&](double X,double a){return a+s*((a-X)/z-a/focal_mm);};
        return {hit(object.x,pupil.x),hit(object.y,pupil.y)};
    }
    // First dark ring diameter at infinity focus; not a combined PSF.
    double airy_infinity_mm(double wavelength_mm)const{
        validate();if(!std::isfinite(wavelength_mm)||wavelength_mm<=0||wavelength_mm>.01)
            throw std::invalid_argument("wavelength in millimetres");
        return 2.44*wavelength_mm*f_number;
    }
};
} // namespace book03

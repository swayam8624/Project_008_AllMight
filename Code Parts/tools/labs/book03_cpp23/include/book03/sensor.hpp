#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <stdexcept>

namespace book03::sensor {
// SI units; the input is irradiance at the sensor, not scene radiance.
inline constexpr double h=6.62607015e-34, c=299792458.0;
inline double photons(double irradiance, double area, double seconds, double wavelength) {
    if (!std::isfinite(irradiance) || irradiance<0 || !std::isfinite(area) || area<=0 ||
        !std::isfinite(seconds) || seconds<0 || !std::isfinite(wavelength) || wavelength<=0)
        throw std::invalid_argument("invalid exposure");
    const double n=irradiance*area*seconds/(h*c/wavelength);
    if (!std::isfinite(n)) throw std::overflow_error("photon count");
    return n;
}
struct Config {
    double qe=0.6, dark_per_second=2, read_sigma=3, full_well=20000, gain=0.2, bias=64;
    unsigned bits=12;
    void validate() const {
        if (!(std::isfinite(qe)&&qe>=0&&qe<=1 && std::isfinite(dark_per_second)&&dark_per_second>=0 &&
              std::isfinite(read_sigma)&&read_sigma>=0 && std::isfinite(full_well)&&full_well>0 &&
              std::isfinite(gain)&&gain>0 && std::isfinite(bias)&&bias>=0 && bits>=1&&bits<=16))
            throw std::invalid_argument("invalid sensor configuration");
    }
};
struct Moments { double signal, dark, variance, snr; };
// Moments before full-well clipping, readout clipping and quantization.
inline Moments moments(const Config& p, double incident, double seconds) {
    p.validate();
    if (!std::isfinite(incident)||incident<0||!std::isfinite(seconds)||seconds<0)
        throw std::invalid_argument("invalid measurement");
    const double s=p.qe*incident, d=p.dark_per_second*seconds, v=s+d+p.read_sigma*p.read_sigma;
    if (!std::isfinite(v)||s+d>1e8) throw std::overflow_error("outside simulator count domain");
    return {s,d,v,v>0?s/std::sqrt(v):0};
}
inline std::uint32_t adc(const Config& p, double electrons) {
    p.validate();
    if (!std::isfinite(electrons)) throw std::invalid_argument("invalid analog value");
    const double top=static_cast<double>((1u<<p.bits)-1u);
    // Analog gain in DN/electron, pedestal in DN; clamp before casting.
    return static_cast<std::uint32_t>(std::round(std::clamp(p.bias+p.gain*electrons,0.0,top)));
}
struct Sample { double charge, stored, analog; std::uint32_t dn; };
template<class Engine> Sample capture(const Config& p, double incident, double seconds, Engine& rng) {
    const auto m=moments(p,incident,seconds);
    // Thinned photon arrivals and independent dark events are Poisson.
    const double mean=m.signal+m.dark;
    const double charge=mean==0?0:static_cast<double>(std::poisson_distribution<long long>(mean)(rng));
    const double stored=std::min(charge,p.full_well);
    const double read=p.read_sigma==0?0:std::normal_distribution<double>(0,p.read_sigma)(rng);
    const double analog=stored+read;
    return {charge,stored,analog,adc(p,analog)};
}
} // namespace book03::sensor

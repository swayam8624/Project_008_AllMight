// Book 03 P05 — the "missing triangle" numerical observatory.
//
// Compare a naive binary64 determinant with an EXACT SIGN for the represented
// input points. The exact sign comes solely from exact_orient2d.hpp and the
// old bounded-integer P03 oracle; SVG and CSV are *visualizations of outputs*
// rather than competing geometry implementations.
//
// Commands:
//   book03_precision inspect
//   book03_precision point ax ay bx by cx cy
//   book03_precision csv FILE.csv
//   book03_precision svg FILE.svg
//   book03_precision repl
//
// Example fixture: A=(0,0), B=(1e16,1e16+2), C=(1e16+2,1e16+4).
// Naive determinant == 0; exact signed determinant == -4 (sign -1).
//
// This program performs no time measurement or marketing "speedup" claims.
// A displayed exact sign is not the same as exact physical measurement.

#include "book03/exact_orient2d.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

using book03::ExactOrientPath;
using book03::ExactOrientResult;
using book03::Point2;

struct Case {
    const char* name{};
    Point2 a{},b{},c{};
    const char* explanation{};
};

[[nodiscard]] std::array<Case,5> fixtures() {
    constexpr double n=10'000'000'000'000'000.0;
    const double tiny=std::numeric_limits<double>::denorm_min();
    const double huge=std::numeric_limits<double>::max();
    return {{
        {"Product cancellation", {0,0},{n,n+2},{n+2,n+4},
         "Exact represented-input D=-4; naive double D=0."},
        {"Subnormal underflow", {0,0},{tiny,0},{0,tiny},
         "Exact D is positive but cannot fit a nonzero double."},
        {"Product overflow", {0,0},{huge,0},{0,huge},
         "Exact D is positive, naive magnitude overflows."},
        {"Input rounding", {0,0},{n,n},{n+1,n+1},
         "N+1 rounded to N before either predicate began."},
        {"Simple reference", {0,0},{6,0},{0,4},
         "Ordinary double and bounded int64 signs agree."}
    }};
}

[[nodiscard]] int naive_sign(double x) noexcept {
    // NaN compares false to 0. We do NOT interpret NaN as zero in reports:
    // its separate status is "not comparable" rather than a valid sign.
    return (x>0)-(x<0);
}

[[nodiscard]] const char* sign_name(int sign) {
    switch(sign) {
        case -1: return "negative";
        case 0: return "zero";
        case +1: return "positive";
        default: return "invalid";
    }
}

[[nodiscard]] const char* path_name(ExactOrientPath path) {
    return path==ExactOrientPath::BoundedInteger ?
        "bounded-integer" : "binary64-dyadic";
}

[[nodiscard]] double naive_orientation(Point2 a,Point2 b,Point2 c) {
    return (b.x-a.x)*(c.y-a.y) -
           (b.y-a.y)*(c.x-a.x);
}

[[nodiscard]] std::string naive_state(double x, int exact) {
    if(std::isnan(x)) return "NaN: uncertifiable";
    if(!std::isfinite(x)) return "overflow: magnitude unavailable";
    if(x==0.0 && exact!=0) return "LOST nonzero sign";
    if(naive_sign(x)!=exact) return "WRONG sign";
    return "sign agrees";
}

void display(std::ostream& out,const Case& fixture) {
    const auto result=book03::orient_binary64(
        fixture.a,fixture.b,fixture.c);
    const double naive=naive_orientation(
        fixture.a,fixture.b,fixture.c);
    out << std::setprecision(std::numeric_limits<double>::max_digits10);
    out << "Case: " << fixture.name << "\n"
        << "A=(" << fixture.a.x << "," << fixture.a.y << ") "
        << "B=(" << fixture.b.x << "," << fixture.b.y << ") "
        << "C=(" << fixture.c.x << "," << fixture.c.y << ")\n"
        << "Naive binary64 determinant = " << naive << "\n"
        << "Naive decision = ";
    if(std::isfinite(naive)) out << sign_name(naive_sign(naive)) << "\n";
    else out << "not certified (nonfinite)\n";
    out << "Exact represented-input decision = "
        << sign_name(result.sign) << " (" << result.sign << ")\n"
        << "Exact route = " << path_name(result.path) << "\n"
        << "Diagnostic = " << naive_state(naive,result.sign) << "\n"
        << "Why: " << fixture.explanation << "\n";
}

[[nodiscard]] double finite_arg(std::string_view input) {
    const std::string word{input};
    std::size_t consumed=0;
    const double x=std::stod(word,&consumed);
    if(consumed!=word.size() || !std::isfinite(x))
        throw std::invalid_argument("requires a finite numeric coordinate");
    return x;
}

[[nodiscard]] Case from_args(
    std::string_view ax,std::string_view ay,
    std::string_view bx,std::string_view by,
    std::string_view cx,std::string_view cy) {
    return {"Reader coordinates",
        {finite_arg(ax),finite_arg(ay)},
        {finite_arg(bx),finite_arg(by)},
        {finite_arg(cx),finite_arg(cy)},
        "Exact sign is for the parsed and STORED doubles."};
}

[[nodiscard]] std::array<Case,6> report_rows(const Case& current) {
    const auto known=fixtures();
    return {{
        current,
        known[0],known[1],known[2],known[3],known[4]
    }};
}

void write_csv(std::ostream& out,const Case& current) {
    // Fixture descriptions have no commas; all fields are simple literals.
    // Numeric columns print 17 significant digits for round-trippable
    // binary64 values on the supported IEEE binary64 platforms.
    out.imbue(std::locale::classic());
    out << std::setprecision(std::numeric_limits<double>::max_digits10);
    out << "case,ax,ay,bx,by,cx,cy,naive_determinant,naive_status,"
           "exact_sign,exact_path\n";
    for(const Case& x:report_rows(current)) {
        const auto exact=book03::orient_binary64(x.a,x.b,x.c);
        const double naive=naive_orientation(x.a,x.b,x.c);
        out << x.name << ","
            << x.a.x << "," << x.a.y << ","
            << x.b.x << "," << x.b.y << ","
            << x.c.x << "," << x.c.y << ","
            << naive << ","
            << (std::isfinite(naive) ? sign_name(naive_sign(naive)) :
                "nonfinite") << ","
            << exact.sign << "," << path_name(exact.path) << "\n";
    }
    if(!out) throw std::runtime_error("CSV stream write failure");
}

void write_svg(std::ostream& out,const Case& current) {
    // 6 genuinely computed decision cards, not a fabricated benchmark.
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
           "width=\"1050\" height=\"690\" viewBox=\"0 0 1050 690\" "
           "role=\"img\" aria-labelledby=\"title desc\">\n"
        << "<title id=\"title\">Exact orientation versus naive binary64</title>\n"
        << "<desc id=\"desc\">A six-row comparison computed from the C++23 "
           "geometry source. Each row names the stored input case, the "
           "naive floating orientation status, and the exact signed "
           "orientation of represented binary64 coordinates. It includes "
           "product cancellation, subnormal underflow, product overflow, "
           "input rounding and a simple baseline. These are diagnostic "
           "cases, not measured benchmark rates.</desc>\n"
        << "<rect width=\"1050\" height=\"690\" rx=\"18\" fill=\"#fbfaf7\"/>\n"
        << "<text x=\"29\" y=\"37\" font-family=\"sans-serif\" "
           "font-size=\"24\" font-weight=\"600\" fill=\"#284754\">"
           "The truth is in the sign, not the size of the number</text>\n"
        << "<text x=\"29\" y=\"59\" font-family=\"sans-serif\" "
           "font-size=\"13\" fill=\"#678087\">"
           "Six exact-input decisions. No invented timing or speedup data.</text>\n";

    const auto all=report_rows(current);
    for(std::size_t i=0;i<all.size();++i) {
        const Case& sample=all[i];
        const ExactOrientResult exact=book03::orient_binary64(
            sample.a,sample.b,sample.c);
        const double naive=naive_orientation(
            sample.a,sample.b,sample.c);
        const bool naive_agrees=std::isfinite(naive) &&
                                 naive_sign(naive)==exact.sign;
        const int y=90+static_cast<int>(i)*93;
        out << "<rect x=\"24\" y=\"" << y << "\" width=\"1001\" "
               "height=\"83\" rx=\"11\" fill=\""
            << (i%2==0?"#eef3f0":"#f3f4f1") << "\"/>\n";
        out << "<text x=\"42\" y=\"" << y+26 << "\" "
               "font-family=\"sans-serif\" font-size=\"16\" "
               "font-weight=\"600\" fill=\"#294b57\">"
            << sample.name << "</text>\n";
        out << "<text x=\"42\" y=\"" << y+52 << "\" "
               "font-family=\"sans-serif\" font-size=\"12\" "
               "fill=\"#637780\">" << sample.explanation << "</text>\n";
        out << "<text x=\"42\" y=\"" << y+72 << "\" "
               "font-family=\"monospace\" font-size=\"11\" "
               "fill=\"#536d76\">Path: " << path_name(exact.path)
            << "</text>\n";

        out << "<rect x=\"603\" y=\"" << y+11 << "\" width=\"180\" "
               "height=\"57\" rx=\"9\" fill=\""
            << (naive_agrees?"#e0ede3":"#f4ddcf") << "\"/>\n";
        out << "<text x=\"616\" y=\"" << y+32 << "\" "
               "font-family=\"sans-serif\" font-size=\"13\" "
               "fill=\"#546d74\">Naive floating</text>\n"
            << "<text x=\"616\" y=\"" << y+52 << "\" "
               "font-family=\"monospace\" font-size=\"14\" fill=\""
            << (naive_agrees?"#2e755e":"#a86145") << "\">"
            << (std::isfinite(naive)?sign_name(naive_sign(naive)):
                                    "nonfinite")
            << "</text>\n";
        out << "<rect x=\"803\" y=\"" << y+11 << "\" width=\"205\" "
               "height=\"57\" rx=\"9\" fill=\"#d4ece3\"/>\n"
            << "<text x=\"817\" y=\"" << y+32 << "\" "
               "font-family=\"sans-serif\" font-size=\"13\" "
               "fill=\"#516c67\">Exact stored sign</text>\n"
            << "<text x=\"817\" y=\"" << y+52 << "\" "
               "font-family=\"monospace\" font-size=\"15\" "
               "font-weight=\"600\" fill=\"#176b5e\">"
            << sign_name(exact.sign) << "</text>\n";
    }
    out << "</svg>\n";
    if(!out) throw std::runtime_error("SVG stream write failure");
}

template<typename Writer>
void write_file(const std::filesystem::path& file,
                Writer&& fn,const Case& current) {
    std::ofstream out{file,std::ios::binary|std::ios::trunc};
    if(!out) throw std::runtime_error("unable to open report output file");
    fn(out,current);
    out.flush();
    if(!out) throw std::runtime_error("unable to finalize report output");
    std::cout << "Wrote " << file.string() << "\n";
}

void repl() {
    const auto known=fixtures();
    Case current=known[0];
    std::cout << "Book 03 P05 — exact orientation laboratory\n"
                 "Commands: triangle ax ay bx by cx cy | inspect | "
                 "cases | svg FILE | csv FILE | reset | help | quit\n";
    std::string line;
    while(std::cout << "exact> " && std::getline(std::cin,line)) {
        std::istringstream in{line};
        std::string command;
        in >> command;
        if(command=="quit"||command=="exit") break;
        try {
            if(command=="reset") {
                current=known[0];
                std::cout << "Cancellation example restored.\n";
            } else if(command=="help") {
                std::cout << "triangle ax ay bx by cx cy | inspect | "
                             "cases | svg PATH | csv PATH | reset | quit\n";
            } else if(command=="inspect") {
                display(std::cout,current);
            } else if(command=="cases") {
                for(const auto& c:known) {
                    display(std::cout,c);
                    std::cout << '\n';
                }
            } else if(command=="svg"||command=="csv") {
                std::string file,extra;
                if(!(in>>file) || (in>>extra))
                    throw std::invalid_argument("provide one filename");
                if(command=="svg") write_file(file,write_svg,current);
                else write_file(file,write_csv,current);
            } else if(command=="triangle") {
                std::array<std::string,6> values;
                for(auto& v:values) {
                    if(!(in>>v)) throw std::invalid_argument(
                        "triangle needs exactly six coordinates");
                }
                std::string extra;
                if(in>>extra) throw std::invalid_argument(
                    "triangle accepts exactly six numbers");
                const Case next=from_args(
                    values[0],values[1],values[2],
                    values[3],values[4],values[5]);
                (void)book03::orient_binary64(next.a,next.b,next.c);
                current=next;
                display(std::cout,current);
            } else if(!command.empty()) {
                std::cout << "Unknown command; type help.\n";
            }
        } catch(const std::exception& e) {
            std::cout << "Input rejected: " << e.what() << "\n";
        }
    }
}

void usage() {
    std::cerr << "Usage:\n"
                 " book03_precision inspect\n"
                 " book03_precision point ax ay bx by cx cy\n"
                 " book03_precision csv OUT.csv\n"
                 " book03_precision svg OUT.svg\n"
                 " book03_precision repl\n";
}

} // namespace

int main(int argc,char** argv) {
    if(argc<2) { usage(); return 2; }
    try {
        const std::string_view mode{argv[1]};
        const Case default_case=fixtures()[0];
        if(mode=="inspect" && argc==2) {
            for(const auto& c:fixtures()) {
                display(std::cout,c);
                std::cout << '\n';
            }
        } else if(mode=="point" && argc==8) {
            const auto c=from_args(argv[2],argv[3],argv[4],argv[5],
                                   argv[6],argv[7]);
            display(std::cout,c);
        } else if(mode=="csv" && argc==3) {
            write_file(argv[2],write_csv,default_case);
        } else if(mode=="svg" && argc==3) {
            write_file(argv[2],write_svg,default_case);
        } else if(mode=="repl" && argc==2) {
            repl();
        } else {
            usage();
            return 2;
        }
        return 0;
    } catch(const std::exception& e) {
        std::cerr << "Book03/P05 error: "<<e.what()<<"\n";
        return 1;
    }
}

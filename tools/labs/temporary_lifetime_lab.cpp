#include <cassert>
#include <iostream>
#include <string>
#include <string_view>

struct Tracked {
    int value{};
    inline static int alive = 0;

    explicit Tracked(int v) : value(v) { ++alive; }
    Tracked(const Tracked& other) : value(other.value) { ++alive; }
    ~Tracked() { --alive; }
};

Tracked make_tracked(int v) {
    return Tracked{v};
}

int main() {
    assert(Tracked::alive == 0);

    {
        // Binding a temporary object to this local const reference extends
        // that temporary's lifetime to the reference's lifetime.
        const Tracked& ref = make_tracked(42);
        assert(ref.value == 42);
        assert(Tracked::alive == 1);
    }
    assert(Tracked::alive == 0);

    {
        // A string_view is only an observer.  Keeping the owner alive is the
        // contract that makes this view valid.
        std::string owner = "render_graph";
        std::string_view view = owner;
        assert(view == "render_graph");
        owner[0] = 'R';
        assert(view == "Render_graph");
    }

#if 0
    // Intentionally NOT executed: returning a reference to a local object
    // would create a dangling reference.  The source can look plausible while
    // the lifetime contract is already broken.
    const Tracked& bad() {
        Tracked local{7};
        return local;
    }
#endif

    std::cout << "temporary lifetime: 3 suites PASS\n";
}

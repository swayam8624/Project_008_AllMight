#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

struct Tracked {
    // Count live instances so the program can observe the scope in which the
    // temporary exists; this is test instrumentation, not ownership logic.
    int value{};
    inline static int alive = 0;

    explicit Tracked(int v) : value(v) { ++alive; }
    // A copy creates another object and therefore another live instance.
    Tracked(const Tracked& other) : value(other.value) { ++alive; }
    // Destruction marks the end of this object's lifetime.
    ~Tracked() { --alive; }
};

Tracked make_tracked(int v) {
    // Return an owned value. Copy elision may remove a move, not the object.
    return Tracked{v};
}

void require(bool condition, const char* message) {
    // Keep witness checks active when compiled with NDEBUG.
    if (!condition) throw std::runtime_error(message);
}

int main() {
    require(Tracked::alive == 0, "no tracked object exists before the scope");

    {
        // Binding a temporary object to this local const reference extends
        // that temporary's lifetime to the reference's lifetime.
        const Tracked& ref = make_tracked(42);
        // Direct local binding extends this temporary to the end of the block.
        require(ref.value == 42, "bound temporary retains its value");
        require(Tracked::alive == 1, "one bound temporary is alive");
    }
    // The temporary is destroyed when its extending reference leaves scope.
    require(Tracked::alive == 0, "bound temporary dies at the block boundary");

    {
        // A string_view is only an observer.  Keeping the owner alive is the
        // contract that makes this view valid.
        std::string owner = "render_graph";
        // The view stores an address and length; owner supplies the storage.
        std::string_view view = owner;
        require(view == "render_graph", "view initially observes owner bytes");
        // Character replacement preserves this string's storage allocation.
        owner[0] = 'R';
        require(view == "Render_graph", "view observes the owner's update");
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

    // The dangling example above is disabled: reading it would be undefined.
    std::cout << "temporary lifetime: 2 executable suites PASS (dangling case is excluded)\n";
}



class Adapt
{
    public:
    Adapt(int mn, int mx)
        : min(mn), max(mx)
    { };

    ~Adapt() = default;

    float operator () (int in)
    {
        float a = in - min;
        float b = max - min;
        return a/b * 100.0f;
    };

    int min;
    int max;
};

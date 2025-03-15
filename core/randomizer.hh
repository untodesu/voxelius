#ifndef CORE_RANDOMIZER_HH
#define CORE_RANDOMIZER_HH 1
#pragma once

template<typename T>
class Randomizer final {
public:
    explicit Randomizer(void);
    explicit Randomizer(std::uint64_t seed);
    virtual ~Randomizer(void) = default;
    void add(const T &value);
    const T &get(void);
    void clear(void);

private:
    std::vector<T> m_vector;
    std::mt19937_64 m_twister;
    std::uniform_int_distribution<std::size_t> m_dist;
};

template<typename T>
inline Randomizer<T>::Randomizer(void)
{
    m_vector.clear();
    m_twister.seed(std::random_device()());
    m_dist = std::uniform_int_distribution<std::size_t>(0, 0);
}

template<typename T>
inline Randomizer<T>::Randomizer(std::uint64_t seed)
{
    m_vector.clear();
    m_twister.seed(seed);
    m_dist = std::uniform_int_distribution<std::size_t>(0, 0);
}

template<typename T>
inline void Randomizer<T>::add(const T &value)
{
    m_vector.push_back(value);
    m_dist = std::uniform_int_distribution<std::size_t>(0, m_vector.size() - 1);
}

template<typename T>
inline const T &Randomizer<T>::get(void)
{
    return m_vector.at(m_dist(m_twister));
}

template<typename T>
inline void Randomizer<T>::clear(void)
{
    m_vector.clear();
    m_dist = std::uniform_int_distribution<std::size_t>(0, 0);
}

#endif /* CORE_RANDOMIZER_HH */

struct Spinlock {
    bool locked = false;

    void acquire();
    void release();
};

#ifndef __MEASURE_H__
#define __MEASURE_H__

class MeasureClass {
    private:
        uint32_t retry;
        uint32_t _publish(float);

    public:
             MeasureClass();
        void setup();
        uint32_t loop();
};

extern MeasureClass Measure;
#endif // __MEASURE_H__
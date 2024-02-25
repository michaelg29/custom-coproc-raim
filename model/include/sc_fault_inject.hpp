
#include <time.h>

#include "systemc.h"

#ifndef SC_FAULT_INJECT_HPP
#define SC_FAULT_INJECT_HPP

/** Representation of variable to be upset. */
struct fault_injectable_variable_t {
    char *name;               // name
    uint8_t *ptr;             // pointer to variable
    uint32_t size;            // size
    uint32_t rand_threshold;  // probability of fault per byte
    uint32_t fault_count;     // fault injection count
    uint32_t pos_fault_count; // possible fault injection count
};

/** Class to inject faults into registered variables. */
class sc_fault_injector; // forward declaration for global singleton
class sc_fault_injector {

    public:

        /** Global singleton instance. */
        static sc_fault_injector injector;

        /** Initializer specifying output file. */
        static void init(double step_size, sc_time_unit time_unit) {
            injector._step_size = step_size;
            injector._step_unit = time_unit;
        }

        /** Configure maximum and minimum probability bounds. Default is 1.0f and 0.0f. */
        static void configure_bounds(float max_prob, float min_prob) {
            if (max_prob > 1.0f) max_prob = 1.0f;
            if (min_prob < 0.0f) min_prob = 0.0f;

            injector._max_prob = max_prob;
            injector._min_prob = min_prob;
        }

        /**
         * Register a variable for fault injection.
         *
         * @param value The variable into which inject faults.
         * @param prob  The probability of fault per byte per simulation step when running `simulate`. In mathematical terms, the total probability looks like: `prob` faults/step / (`_step_size`*`_step_unit` s/step) = `prob` / (`_step_size`*`_step_unit`) faults/s.
         * @param name  The display name of the variable.
         */
        template <class T>
        static void set_injectable(T& value, float prob = 0.01f, char* name = NULL) {
            set_injectable_ptr<T>(&value, 1, prob, name);
        }

        /**
         * Register a variable array for fault injection.
         *
         * @param value The variable into which inject faults.
         * @param n     The number of variables in the array.
         * @param prob  The probability of fault per byte per simulation step when running `simulate`. In mathematical terms, the total probability looks like: `prob` faults/step / (`_step_size`*`_step_unit` s/step) = `prob` / (`_step_size`*`_step_unit`) faults/s.
         * @param name  The display name of the variable.
         */
        template <class T>
        static void set_injectable_ptr(T* value, uint32_t n, float prob = 0.01f, char* name = NULL) {
            // check bounds
            if (prob > injector._max_prob) prob = injector._max_prob;
            if (prob < injector._min_prob) prob = injector._min_prob;

            // generate structure
            fault_injectable_variable_t registration;
            registration.name = name;
            registration.ptr = (uint8_t*)value;
            registration.size = sizeof(T) * n;
            registration.rand_threshold = (uint32_t)(prob * (double)((uint32_t)0xffffffff)); // compute integer value for threshold
            registration.fault_count = 0;
            registration.pos_fault_count = 0;
            injector._var_list.push_back(registration);
        }

        /** Run the simulation, stepping `_step_size` `_step_unit` before testing for injection. Returns the duration. */
        static sc_time simulate(double max_time = -1.0) {
            srand(time(0));

            double time = 0.0;
            sc_time start_time = sc_time_stamp();
            while (injector._enabled) {
                // simulate a step
                sc_start(injector._step_size, injector._step_unit);
                time += injector._step_size;

                // check if done
                if ((max_time > 0.0 && time >= max_time) || !sc_pending_activity_at_current_time()) {
                    break;
                }

                if (!injector._enabled) continue;

                // if enabled, inject faults into registration list
                uint32_t rand_val;
                uint8_t fault_bit;
                for (fault_injectable_variable_t &var : injector._var_list) {
                    uint8_t *ptr = var.ptr;
                    uint32_t n_bytes = var.size;
                    while (n_bytes) {
                        // probability of random fault
                        rand_val = (uint32_t)rand();
                        if (rand_val < var.rand_threshold) {
                            var.pos_fault_count++;

                            // determine which bit to upset
                            fault_bit = rand_val & 0x7;
                            fault_bit = 0b1 << fault_bit;

                            // perform mask
                            if ((uint32_t)rand() & 0x8) {
                                // 50% chance of possible 0 --> 1 fault
                                if (!(*ptr & fault_bit)) var.fault_count++;
                                *ptr |= fault_bit;
                            }
                            else {
                                // 50% chance of possible 1 --> 0 fault
                                if (*ptr & fault_bit) var.fault_count++;
                                *ptr ^= fault_bit;
                            }
                        }

                        // increment counter
                        n_bytes--;
                        ptr++;
                    }

                    //std::cout << var.name << " has a fault count of " << var.fault_count << " and a possible fault count of " << var.pos_fault_count << std::endl;
                }
            }

            // if never started (stepper disabled), run until completion
            if (time == 0.0) {
                sc_start();
            }
            sc_time stop_time = sc_time_stamp();

            // print report
            print();

            // return duration
            return stop_time - start_time;
        }

        /** Disable tracing. */
        static void disable() {
            injector._enabled = false;
        }

        /** Enable tracing. */
        static void enable() {
            injector._enabled = true;
        }

        static void print() {
            uint32_t total_faults = 0;
            uint32_t total_size = 0;
            for (fault_injectable_variable_t &var : injector._var_list) {
                total_faults += var.fault_count;
                total_size += var.size;
            }
            std::cout << "Total of " << total_faults << " faults over " << total_size << " bytes." << std::endl;
        }

    private:

        /** Enable switch. */
        bool _enabled = true;

        /** Bounds. */
        float _max_prob = 1.0f;
        float _min_prob = 0.0f;

        /** Simulation step size. */
        double _step_size = 10;
        sc_time_unit _step_unit = SC_NS;

        /** Internal list. */
        std::vector<fault_injectable_variable_t> _var_list;

};

#endif // SC_FAULT_INJECT_HPP

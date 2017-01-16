//
// Copyright (c) 2016 Enrico Kaden & University College London
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#ifndef _PROGRESS_H
#define _PROGRESS_H

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <thread>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "darray.h"

namespace smt {

namespace {

std::string getenv(const std::string& var) {
	const char* tmp = std::getenv(var.c_str());
	if(tmp == nullptr) {
		return {};
	} else {
		return std::string(tmp);
	}
}

} // (anonymous)

class progress {
public:
	progress(const unsigned long int& n, const std::string& name = "Progress"):
		_delay(100l),
		_n(n),
		_name(name),
		_i(init_i()),
		_t(verbose()? std::thread{&progress::run, this} : std::thread{}) {
	}

	void operator++() {
#ifdef _OPENMP
		++_i(omp_get_thread_num());
#else
		++_i(0);
#endif
	}

	~progress() {
		if(_t.joinable()) {
			_t.join();
		}
	}

private:
	const long int _delay;
	const unsigned long int _n;
	const std::string _name;
	smt::darray<unsigned long int, 1u> _i;
	std::thread _t;

	smt::darray<unsigned long int, 1u> init_i() const {
#ifdef _OPENMP
		smt::darray<unsigned long int, 1u> i{static_cast<unsigned int>(std::max(omp_get_max_threads(), 1))};
#else
		smt::darray<unsigned long int, 1u> i{1u};
#endif
		i = 0ul;

		return i;
	}

	void run() const {
		while(true) {
			const unsigned long int sum_i = std::accumulate(std::begin(_i), std::end(_i), 0ul);
			const float prgs = std::min(float(sum_i)/_n, 1.0f);
			const unsigned int pos = std::floor(50u*prgs);

			std::cerr << std::string(_name, 0ul, 18ul) << ' ' << std::string(20ul-std::min(_name.length(), 18ul), '.') << " [";
			for(unsigned int ii = 0u; ii < 50u; ++ii) {
				if(ii < pos) {
					std::cerr << "=";
				} else if(ii == pos) {
					std::cerr << ">";
				} else {
					std::cerr << " ";
				}
			}
			std::cerr << "] " << std::setw(3) << std::floor(100.0f*prgs) << "%\r";

			if(sum_i < _n) {
				std::cerr.flush();
				std::this_thread::sleep_for(std::chrono::milliseconds(_delay));
			} else {
				std::cerr << std::endl;
				std::cerr.flush();
				break;
			}
		}
	}

	bool verbose() const {
		const std::string val{smt::getenv("SMT_QUIET")};
		if(val == "true" || val == "TRUE" || std::atoi(val.c_str()) > 0) {
			return false;
		} else {
			return true;
		}
	}
};

} // smt

#endif // _PROGRESS_H

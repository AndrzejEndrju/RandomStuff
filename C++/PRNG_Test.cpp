#include <iostream>
#include <random>
#include <vector>
#include <cmath> 
#include <limits>

template<typename T>
class PRNG_Tester
{
public:

	PRNG_Tester(size_t InAmountOfBins, T InMinVal, T InMaxVal)
		: AmountOfBins(InAmountOfBins), MinVal(InMinVal), MaxVal(InMaxVal)
	{
		if (!AmountOfBins)
		{
			std::cout << "Error: AmountOfBins passed to Tester ctor is equal to 0u.\n";
			return;
		}

		if (MaxVal < MinVal)
		{
			std::cout << "Error: MaxVal passed to Tester ctor is lesser to MinVal passed to the same function.\n";
			return;
		}

		if (MaxVal == MinVal)
		{
			std::cout << "Error: MaxVal passed to Tester ctor is equal to MinVal passed to the same function.\n";
			return;
		}

		Bins.reserve(AmountOfBins);

		for (size_t i = 0u; i < AmountOfBins; ++i)
		{
			Bins.push_back(0.f);
		}

		Diff = (MaxVal - MinVal) / AmountOfBins;

		CanWork = true;
	}

	std::vector<T> Bins;
	size_t AmountOfBins = 0u;
	size_t AmountOfSamples = 0u;
	T MinVal;
	T MaxVal;
	T Diff;
	bool CanWork = false;

	bool Test(T Sample)
	{
		++AmountOfSamples;

		if (!CanWork)
		{
			std::cout << "Error: Tester was incorrectly initialized therefore the test cannot run.\n";
			return false;
		}

		if (Sample >= MinVal && Sample <= MaxVal)
		{
			for (size_t i = 1u; i <= Bins.size(); ++i)
			{
				if (Sample >= (i != 1u ? Diff * (i - 1u) + MinVal : MinVal)
					&& Sample <= Diff * i + MinVal)
				{
					++Bins[i - 1u];
					break;
				}
			}

		}
		else
		{
			std::cout << "Error: Sample passed to PRNG_Test() function has invalid sample equal to " << Sample << " where min val = " << MinVal
				<< " and max val = " << MaxVal << "\n";
			return false; // here we can also add flag and set it instead of return and later just print either everything or only failed etc
		}

		return true;
	}

	void Print()
	{
		if (!CanWork)
		{
			std::cout << "Error: Tester was incorrectly initialized therefore the print cannot run.\n";
			return;
		}

		for (size_t i = 1u; i <= Bins.size(); ++i)
		{
			std::cout << (i != 1u ? Diff * (i - 1u) + MinVal : MinVal) << " - " << Diff * i + MinVal << " Bin[" << i - 1u << "] = " << (Bins[i - 1u] / AmountOfSamples) * (AmountOfBins / Diff) << "\n";
		}
	}
};

int main()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distrib(0.f, 1.f);

	PRNG_Tester<float>* test = new PRNG_Tester<float>(5u, 0.f, 1.f);

	for (size_t i = 0u; i < 400000; ++i)
	{
		if (!test->Test(distrib(gen)))
		{
			break;
		}
	}

	test->Print();

	return 0;
}
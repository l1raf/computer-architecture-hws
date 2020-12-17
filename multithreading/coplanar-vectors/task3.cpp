// Курбанова Сабият Магомедовна
// Группа БПИ193
// Вариант 10

#include <iostream>
#include <vector>
#include <fstream>
#include <thread>

constexpr auto THREADS_COUNT = 8;

struct Vector
{
public:
	double X;
	double Y;
	double Z;

	Vector() 
	{
		X = 0;
		Y = 0;
		Z = 0;
	}

	Vector(double x, double y, double z)
	{
		X = x;
		Y = y;
		Z = z;
	}

	/*
	* Checks if this vector and two other are coplanar by
	* checking if scalar triple product is equal to 0
	*/
	bool isCoplanar(Vector v2, Vector v3)
	{
		double p[3] = { v2.Y * v3.Z - v2.Z * v3.Y,
						v2.Z * v3.X - v2.X * v3.Z,
						v2.X * v3.Y - v2.Y * v3.X };

		return X * p[0] + Y * p[1] + Z * p[2] == 0;
	}
};

std::ostream& operator<<(std::ostream& strm, const Vector& v) {
	return strm << "(" << v.X << ", " << v.Y << ", " << v.Z << ")";
}

/*
* Reads vectors from file
*/
std::vector<Vector> getVectorsFromFile(std::string path)
{
	std::vector<Vector> vectors;
	std::fstream in(path, std::ios::in);

	if (in.is_open())
	{
		int vectorsCount = 0;
		double x;
		double y;
		double z;

		while (in >> x >> y >> z)
		{
			Vector currentVector(x, y, z);
			vectors.push_back(currentVector);
		}

		in.close();
	}

	return vectors;
}

/*
* Finds all triplets of coplanar vectors
*/
void findTriplets(std::vector<Vector> vectors, std::vector<std::vector<Vector>>& coplanarVectors, int threads, int start)
{
	for (int k = start; k < vectors.size(); k += threads)
	{
		for (int i = k + 1; i < vectors.size(); i++)
		{
			for (int j = i + 1; j < vectors.size(); j++)
			{
				if (vectors[k].isCoplanar(vectors[i], vectors[j]))
				{
					std::vector<Vector> triplet;

					triplet.push_back(vectors[k]);
					triplet.push_back(vectors[i]);
					triplet.push_back(vectors[j]);

					coplanarVectors.push_back(triplet);
				}
			}
		}
	}
}

/*
* Prints result to console and output file
*/
void printResult(std::vector<std::vector<Vector>> coplanarVectors, std::string outputPath)
{
	std::ofstream out(outputPath, std::ios::out);

	if (coplanarVectors.size() == 0)
	{
		std::cout << "There are no coplanar vectors." << std::endl;;
		return;
	}

	std::cout << "Coplanar vectors: " << std::endl;

	for (int i = 0; i < coplanarVectors.size(); i++)
	{
		std::cout << i + 1 << ". " << coplanarVectors[i][0] << ", " << coplanarVectors[i][1] << ", " << coplanarVectors[i][2] << std::endl;

		if (out.is_open())
		{
			out << coplanarVectors[i][0] << ", " << coplanarVectors[i][1] << ", " << coplanarVectors[i][2] << std::endl;
		}
	}
}

int main(int argc, char** argv)
{
	std::vector<Vector> vectors;
	std::vector<std::vector<Vector>> coplanarVectors[THREADS_COUNT];
	std::vector<std::vector<Vector>> res;

	if (argc < 3)
	{
		std::cout << "Wrong number of args." << std::endl;
		return -1;
	}

	vectors = getVectorsFromFile(argv[1]);
	std::thread* thr[THREADS_COUNT];

	// Create threads
	for (int i = 0; i < THREADS_COUNT; i++)
	{
		thr[i] = new std::thread{ findTriplets, vectors, std::ref(coplanarVectors[i]), THREADS_COUNT, i };
	}

	for (int i = 0; i < THREADS_COUNT; i++)
	{
		thr[i]->join();
		res.insert(std::end(res), std::begin(coplanarVectors[i]), std::end(coplanarVectors[i]));
		delete thr[i];
	}

	printResult(res, argv[2]);
}

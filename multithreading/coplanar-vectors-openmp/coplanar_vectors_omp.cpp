// Курбанова Сабият Магомедовна
// Группа БПИ193
// Вариант 10

#include <iostream>
#include <vector>
#include <fstream>
#include "omp.h"

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

	/// <summary>
	/// Checks if this vector and two other are coplanar by
	/// checking if scalar triple product is equal to 0
	/// </summary>
	/// <param name="v2">second vector</param>
	/// <param name="v3">third vector</param>
	/// <returns>if vectors are coplanar</returns>
	bool isCoplanar(Vector v2, Vector v3)
	{
		double p[3] = { v2.Y * v3.Z - v2.Z * v3.Y,
						v2.Z * v3.X - v2.X * v3.Z,
						v2.X * v3.Y - v2.Y * v3.X };

		return X * p[0] + Y * p[1] + Z * p[2] == 0;
	}
};

/// <summary>
/// Reads vectors from file
/// </summary>
/// <param name="path">path to input file</param>
/// <param name="vectors">vector of Vectors</param>
/// <returns>if reading was successful</returns>
bool getVectorsFromFile(std::string path, std::vector<Vector>& vectors)
{
	std::fstream in(path, std::ios::in);
	double x;
	double y;
	double z;

	if (in.is_open())
	{
		in >> x >> y >> z;

		while (!in.eof())
		{
			if (in.fail() && !in.eof())
				return false;

			Vector currentVector(x, y, z);
			vectors.push_back(currentVector);

			in >> x >> y >> z;
		}

		in.close();
		return true;
	}
	return false;
}

/// <summary>
/// Finds all triplets of coplanar vectors (thread function)
/// </summary>
/// <param name="vectors">input vectors</param>
/// <param name="k">vector's index</param>
void findTriplets(std::vector<Vector> vectors, int k)
{
	for (int i = k + 1; i < vectors.size(); i++)
	{
		for (int j = i + 1; j < vectors.size(); j++)
		{
			if (vectors[k].isCoplanar(vectors[i], vectors[j]))
			{
				printf("thread %d: (%g, %g, %g), (%g, %g, %g), (%g, %g, %g)\n", 
					omp_get_thread_num(), 
					vectors[k].X, vectors[k].Y, vectors[k].Z, 
					vectors[i].X, vectors[i].Y, vectors[i].Z,
					vectors[j].X, vectors[j].Y, vectors[j].Z);
			}
		}
	}
}

int main(int argc, char** argv)
{
	std::vector<Vector> vectors;

	if (argc != 2)
	{
		std::cout << "Wrong number of args." << std::endl;
		return -1;
	}

	if (!getVectorsFromFile(argv[1], vectors))
	{
		std::cout << "Wrong data in file." << std::endl;
		return -1;
	}

	if (vectors.size() < 3)
	{
		std::cout << "There are no coplanar vectors." << std::endl;
		return 0;
	}

	std::cout << omp_get_max_threads() << " threads are used" << std::endl;
	omp_set_num_threads(omp_get_max_threads());

#pragma omp parallel for 
	for (int i = 0; i < vectors.size(); i++)
		findTriplets(vectors, i);
}

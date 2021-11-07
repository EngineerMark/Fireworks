#pragma once

#include "olcPixelGameEngine.h"

#define _USE_MATH_DEFINES
#include <math.h>

namespace FW {

#pragma region Helpers
	/// <summary>
	/// We could use the vector classes from olcPGE, but this makes porting easier,
	/// rendering still uses the olc::vectors
	/// </summary>
	struct vector {
		double x = 0;
		double y = 0;

		/// <summary>
		/// Returns a random vector between -1 and 1
		/// </summary>
		/// <returns></returns>
		static vector rand() {
			return {
				((double)(std::rand() % 100 - 50)) * 0.01,
				((double)(std::rand() % 100 - 50)) * 0.01
			};
		}

		/// <summary>
		/// Returns a random direction
		/// </summary>
		/// <returns></returns>
		static vector randAngle() {
			double a = std::rand() % 360;
			double a_rad = a * M_PI / 180;
			return {
				(double)std::cos(a_rad),
				(double)std::sin(a_rad)
			};
		}

		void Add(vector v) {
			x += v.x;
			y += v.y;
		}

		void Add(double i) {
			x += i;
			y += i;
		}

		void Mul(vector v) {
			x *= v.x;
			y *= v.y;
		}

		void Mul(double i) {
			x *= i;
			y *= i;
		}

		vector operator *(const vector rhs) { return { this->x * rhs.x, this->y * rhs.y }; };
		vector operator *(const double rhs) { return { this->x * rhs, this->y * rhs }; };
		vector operator *=(const vector rhs) { return { this->x *= rhs.x, this->y *= rhs.y }; };
		vector operator *=(const double rhs) { return { this->x *= rhs, this->y *= rhs }; };

		vector operator +(const vector rhs) { return { this->x + rhs.x, this->y + rhs.y }; };
		vector operator +(const double rhs) { return { this->x + rhs, this->y + rhs }; };
		vector operator +=(const vector rhs) { return { this->x += rhs.x, this->y += rhs.y }; };
		vector operator +=(const double rhs) { return { this->x += rhs, this->y += rhs }; };

		vector operator -(const vector rhs) { return { this->x - rhs.x, this->y - rhs.y }; };
		vector operator -(const double rhs) { return { this->x - rhs, this->y - rhs }; };
		vector operator -=(const vector rhs) { return { this->x -= rhs.x, this->y -= rhs.y }; };
		vector operator -=(const double rhs) { return { this->x -= rhs, this->y -= rhs }; };

		vector operator /(const vector rhs) { return { this->x / rhs.x, this->y / rhs.y }; };
		vector operator /(const double rhs) { return { this->x / rhs, this->y / rhs }; };
		vector operator /=(const vector rhs) { return { this->x /= rhs.x, this->y /= rhs.y }; };
		vector operator /=(const double rhs) { return { this->x /= rhs, this->y /= rhs }; };

#ifdef OLC_PGE_DEF
		olc::vd2d ConvertToOlcVD() {
			return { x,y };
		}

		olc::vi2d ConvertToOlcVI() {
			return { (int)x, (int)y };
		}
#endif
	};
#pragma endregion

	static vector G_GRAVITY = { 0, 0.2 };

	class Firework;

	class Particle {
	public:
		vector position;
		vector velocity = { 0, 0 };
		vector acceleration;
		double life_span = 1;

		Firework* firework = NULL;

		Particle(vector position, Firework* firework) {
			this->position = position;

			if (firework != NULL) {
				velocity.y = 0 - (rand() % 50 + 100);

				this->firework = firework;
			}
			else {
				velocity = vector::randAngle();
				velocity *= ((rand() % 40 + 20) * 0.1);
				velocity *= rand() % 50;
			}
		}

		void ApplyForce(vector force) {
			acceleration.Add(force);
		}

		bool IsFinished() {
			return life_span == 0;
		}

		void Update(float fElapsedTime) {
			if (firework == NULL) {
				velocity *= 0.95;
				life_span -= fElapsedTime;

				life_span = std::max(life_span, 0.0);
			}

			velocity.Add(acceleration);
			position.Add(velocity * 0.01);

			acceleration.Mul(0);
		}

#ifdef OLC_PGE_DEF
		void Show(olc::PixelGameEngine& renderer) {
			olc::Pixel p(255, 255, 255, life_span * 255);
			renderer.Draw(position.ConvertToOlcVI(), p);
			//renderer.FillCircle(position.ConvertToOlcVI(), 2, p);
		}
#endif
	};

	class Firework {
	public:
		std::vector<Particle*> particles;
		Particle* firework_particle;

		bool exploded = FALSE;

		Firework() : Firework({ 0,0 }) {

		}

		Firework(vector position) {
			firework_particle = new Particle(position, this);
		}

		void Cleanup() {
			delete firework_particle;

			if (particles.size() > 0) {
				for (auto& p : particles)
					delete p;

				particles.clear();
			}
		}

		void Explode() {
			for (size_t i = 0; i < 100; i++)
			{
				Particle* p = new Particle(firework_particle->position, NULL);
				particles.push_back(p);
			}
		}

		bool IsFinished() {
			if (particles.size() > 0) {
				for (auto& p : particles) {
					if (!p->IsFinished())
						return false;
				}
				return true;
			}

			return false;
		}

		void Update(float fElapsedTime) {
			if (!exploded) {
				firework_particle->ApplyForce(G_GRAVITY);
				firework_particle->Update(fElapsedTime);

				if (firework_particle->velocity.y >= 0) {
					exploded = TRUE;
					Explode();
				}
			}

			if (particles.size() > 0)
				for (auto& p : particles) {
					p->ApplyForce(G_GRAVITY);
					p->Update(fElapsedTime);
				}
		}

#ifdef OLC_PGE_DEF
		void Show(olc::PixelGameEngine& renderer) {
			if (!exploded)
				firework_particle->Show(renderer);

			if (particles.size() > 0)
				for (auto& p : particles)
					p->Show(renderer);
		}
#endif
	};

	class FireworkShow {
	public:
		std::vector<Firework*> fireworks;
		olc::PixelGameEngine& renderer;

		const double UPDATE_RATE = 1 / 120;
		double _fElapsedTime = 0;

		FireworkShow(olc::PixelGameEngine& renderer) : renderer(renderer) {
		}

		void Cleanup() {
			for (auto& p : fireworks) {
				p->Cleanup();
				delete p;
			}
			fireworks.clear();
		}

		void Update(float fElapsedTime) {
			_fElapsedTime += fElapsedTime;
			if (_fElapsedTime > UPDATE_RATE) {
				/*for (auto& p : fireworks) {
					p->Update(_fElapsedTime);
				}*/
				if (fireworks.size() > 0) {
					for (unsigned i = fireworks.size() - 1; i > 0; i--)
					{
						Firework& f = *fireworks[i - 1];
						f.Update(_fElapsedTime);

						if (f.IsFinished()) {
							f.Cleanup();
							delete& f;
							fireworks.erase(fireworks.begin() + i - 1);

						}
					}
				}

				//std::cout << "Firework buffer size: " + std::to_string(fireworks.size()) << std::endl;

				//if (rand() % 100 <= 10)
					fireworks.push_back(new Firework({ (double)(rand() % renderer.ScreenWidth()), (double)renderer.ScreenHeight() - 2 }));

				_fElapsedTime = 0;
			}
		}

		void Show() {
			for (auto& p : fireworks) {
				p->Show(renderer);
			}
		}
	};
};
#include <easy_sdl.h>
#include <math.h>

typedef struct			s_vec
{
	float				x;
	float				y;
	float				z;
}						t_vec;

typedef struct			s_ray
{
	t_vec				start;
	t_vec				dir;
}						t_ray;

typedef struct			s_sphere
{
	t_vec				pos;
	float				size;
}						t_sphere;

typedef struct			s_data
{
	t_esdl				*esdl;
	SDL_Surface			*surf;
}						t_data;

t_vec				vec_sub(t_vec v1, t_vec v2)
{
	t_vec			ret;

	ret.x = v1.x - v2.x;
	ret.y = v1.y - v2.y;
	ret.z = v1.z - v2.z;
	return (ret);
}

t_vec			scalar_mult(float c, t_vec v)
{
	t_vec			ret;

	ret.x = v.x * c;
	ret.y = v.y * c;
	ret.z = v.z * c;
	return (ret);
}

float				vec_mult_chelou(t_vec v1, t_vec v2)
{
	return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

int					hitsphere(t_ray ray, t_sphere sphere, float t)
{
   // intersection rayon/sphere
   //vecteur dist = s.pos - r.start;

	t_vec dist = vec_sub(sphere.pos, ray.start);

   //float B = r.dir * dist;
   float B = vec_mult_chelou(ray.dir, dist);

   //float D = (B * B) - (dist * dist) + s.size * s.size;
   float D = (B * B) - vec_mult_chelou(dist, dist) + sphere.size * sphere.size;

   if (D < 0.0f)
	 return 0;

   float t0 = B - sqrtf(D);
   float t1 = B + sqrtf(D);
   int retvalue = 0;
   if ((t0 > 0.1f) && (t0 < t))
   {
	 t = t0;
	 retvalue = 1;
   }
   if ((t1 > 0.1f) && (t1 < t))
   {
	 t = t1;
	 retvalue = 1;
   }
   return retvalue;
}

void				raytrace(t_data *data)
{
	t_sphere		sphere = {{320.0, 240.0, 0.0}, 100};
	t_ray			ray = {{0, 0, -10000.0f}, { 0.0f, 0.0f, 1.0f}};
	float			max = 20000.0f;


	for (int y = 0; y < SDL_RY; y++)
	{
		for (int x = 0; x < SDL_RX; x++)
		{
			ray.start.x = x;
			ray.start.y = y;


			if (hitsphere(ray, sphere, max) == 1)
				esdl_put_pixel(data->surf, x, y, 0xFFFFFFFF);
		}
	}
}

void					display(t_data *data)
{

	SDL_Texture			*texture = SDL_CreateTextureFromSurface(data->esdl->en.ren, data->surf);
	SDL_RenderClear(data->esdl->en.ren);
	SDL_RenderCopy(data->esdl->en.ren, texture, NULL, NULL);
	SDL_RenderPresent(data->esdl->en.ren);
	SDL_DestroyTexture(texture);
}

void				init(t_data *data)
{
	data->surf = esdl_create_surface(SDL_RX, SDL_RY);
}

void				quit(t_data *data)
{
	SDL_FreeSurface(data->surf);
}

int					main(int argc, char **argv)
{
	t_data			data;
	t_esdl			esdl;

	data.esdl = &esdl;

	if (esdl_init(&esdl, 640, 480, "Engine") == -1)
		return (-1);
	init(&data);

	raytrace(&data);
	display(&data);

	while (esdl.run)
	{
		esdl_update_events(&esdl.en.in, &esdl.run);


		esdl_fps_limit(&esdl);
		esdl_fps_counter(&esdl);
	}
	quit(&data);
	esdl_quit(&esdl);
	(void)argc;
	(void)argv;
	return (0);
}

#include <easy_sdl.h>
#include <rtv1.h>

typedef struct			s_ray
{
	t_vec				start;
	t_vec				dir;
}						t_ray;

typedef struct			s_sphere
{
	t_vec				pos;
	float				rad;
	t_vec				surf_color;
	t_vec				emis_color;
	int					is_light;
}						t_sphere;

typedef struct			s_spheres
{
	int					nb_spheres;
	t_sphere			*spheres;
}						t_spheres;

typedef struct			s_data
{
	t_esdl				*esdl;
	t_spheres			spheres;
	t_spheres			lights;
	SDL_Surface			*surf;
}						t_data;

t_sphere		set_sphere(t_vec pos, float radius, t_vec surf_color)
{
	t_sphere	sphere;

	sphere.pos = pos;
	sphere.rad = radius;
	sphere.surf_color = surf_color;
	sphere.is_light = 0;
	return (sphere);
}

t_sphere		set_light(t_vec pos, float radius, t_vec emis_color)
{
	t_sphere	light;

	light.pos = pos;
	light.rad = radius;
	light.emis_color = emis_color;
	light.is_light = 1;
	return (light);
}

void			debug_sphere(t_sphere sphere)
{
	printf("x = %f y = %f z = %f rad = %f\n", sphere.pos.x, sphere.pos.y, sphere.pos.z, sphere.rad);
	printf("surf_color red = %f green = %f blue = %f\n", sphere.surf_color.x, sphere.surf_color.y, sphere.surf_color.z);
	printf("emis_color red = %f green = %f blue = %f\n", sphere.emis_color.x, sphere.emis_color.y, sphere.emis_color.z);
	printf("\n");
}

void			debug_spheres(t_spheres *spheres)
{
	t_sphere	sphere;

	for (int i = 0; i < spheres->nb_spheres; i++)
	{
		if (spheres->spheres == NULL)
			break ;
		debug_sphere(spheres->spheres[i]);
	}
	printf("\n\n\n");
}

int				init_spheres(const unsigned int nb_spheres, t_spheres *spheres)
{
	spheres->nb_spheres = nb_spheres;
	spheres->spheres = NULL;
	if (!(spheres->spheres = (t_sphere *)malloc(sizeof(t_sphere) * nb_spheres)))
		return (0);
	return (1);
}

int					hitsphere(t_vec rayorig, t_vec raydir, t_sphere sphere, float *t0, float *t1)
{
	t_vec l = vec_sub(sphere.pos, rayorig);
	float tca = dot_product(l, raydir);
	if (tca < 0)
	    return 0;
	float d2 = dot_product(l, l) - tca * tca;
	if (d2 > (sphere.rad * sphere.rad))
	    return 0;
	float thc = sqrtf((sphere.rad * sphere.rad) - d2);
	*t0 = tca - thc;
	*t1 = tca + thc;
	return (1);
}

int				raytrace(t_vec rayorig, t_vec raydir, t_data *data)
{
	float		tnear;
	t_sphere	*sphere = NULL;
	float		t0;
	float		t1;

	tnear = INFINITY;
	for (unsigned i = 0; i < data->spheres.nb_spheres; i++)
	{
	    t0 = INFINITY;
	    t1 = INFINITY;
	    if (hitsphere(rayorig, raydir, data->spheres.spheres[i], &t0, &t1))
	    {
	        if (t0 < 0)
	            t0 = t1;
	        if (t0 < tnear)
	        {
	            tnear = t0;
	            sphere = &(data->spheres.spheres[i])
;	        }
	    }
	}
	if (!sphere)
	    return (0xFFFFFFFF);

	t_vec phit = vec_add(rayorig, vec_mult_f(raydir, tnear));
	t_vec nhit = vec_sub(phit, sphere->pos);
	nhit = vec_normalize(nhit);

	float red, green, blue;
    for (int i = 0; i < data->lights.nb_spheres; i++)
    {
    	if (data->lights.spheres[i].is_light == 1)
    	{
			t_sphere light = data->lights.spheres[i];

			int transmission = 1;
			t_vec lightDirection = vec_sub(light.pos, phit);
			lightDirection = vec_normalize(lightDirection);
			for (unsigned j = 0; j < data->spheres.nb_spheres; j++)
			{
			    if (hitsphere(vec_add(phit, nhit), lightDirection, data->spheres.spheres[j], &t0, &t1) == 1)
			    {
			        transmission = 0;
			        break;
			    }
			}
			red = sphere->surf_color.x * transmission * max(0.0f, dot_product(nhit,
				lightDirection)) * 3.0f;

			green = sphere->surf_color.y * transmission * max(0.0f, dot_product(nhit, lightDirection)) * 3.0f;


			blue = sphere->surf_color.z * transmission * max(0.0f, dot_product(nhit, lightDirection)) * 3.0f;

			red = min(1.0f, red) * 255;
			green = min(1.0f, green) * 255;
			blue = min(1.0f, blue) * 255;
		}
	}



	return ((int)(red) << 24 | (int)(green) << 16 | (int)(blue) << 8 | 255);
}

void
render(t_data *data)
{
	float invWidth = 1.0f / (float)SDL_RX;
	float invHeight = 1.0f / (float)SDL_RY;
	float fov = 30.0f;
	float aspectratio = SDL_RX / (float)SDL_RY;
	float angle = tan(M_PI * 0.5f * fov / 180.0f);

	for (int y = 0; y < SDL_RY; y++)
	{
		for (int x = 0; x < SDL_RX; x++)
		{
            float xx = (2.0f * ((x + 0.5f) * invWidth) - 1.0f) * angle * aspectratio;
            float yy = (1.0f - 2.0f * ((y + 0.5f) * invHeight)) * angle;
            t_vec raydir = {xx, yy, -1};

            raydir = vec_normalize(raydir);
            t_vec rayorig = {0, 0, 0};

            esdl_put_pixel(data->surf, x, y, raytrace(rayorig, raydir, data));
		}
	}
}

void				display(t_data *data)
{

	SDL_Texture		*texture = SDL_CreateTextureFromSurface(data->esdl->en.ren, data->surf);
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

	init_spheres(2, &data.spheres);
	data.spheres.spheres[0] = set_sphere(set_vec(-2.0f, 0.0f, -20.0f), 4.0f, set_vec(1.0f, 0.32f, 0.36f));
	data.spheres.spheres[1] = set_sphere(set_vec(2.0f, 0.0f, -20.0f), 4.0f, set_vec(0.32f, 1.0f, 0.36f));

	debug_spheres(&data.spheres);

	init_spheres(2, &data.lights);
	data.lights.spheres[0] = set_light(set_vec(0.0, -100.0f, -30.0f), 3.0f, set_vec(3, 3, 3));
	data.lights.spheres[1] = set_light(set_vec(2.0, 20.0f, -30.0f), 3.0f, set_vec(3, 3, 3));

	debug_spheres(&data.lights);

	if (esdl_init(&esdl, 640, 480, "Engine") == -1)
		return (-1);
	init(&data);

	render(&data);
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
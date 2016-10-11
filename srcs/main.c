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
	SDL_Surface			*surf;
}						t_data;

typedef struct			s_material {
  float specValue;
  float specPower;
}						t_material;

#define SUPERSAMPLING

t_sphere		set_sphere(t_vec pos, float radius, t_vec surf_color)
{
	t_sphere	sphere;

	sphere.pos = pos;
	sphere.rad = radius;
	sphere.surf_color = surf_color;
	sphere.is_light = 0;
	sphere.emis_color = set_vec(0.0f, 0.0f, 0.0f);
	return (sphere);
}

t_sphere		set_light(t_vec pos, float radius, t_vec emis_color)
{
	t_sphere	light;

	light.pos = pos;
	light.rad = radius;
	light.surf_color = set_vec(0.0f, 0.0f, 0.0f);
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

float			calculateLambert(t_vec phit, t_vec nhit, t_sphere light)
{
	t_vec		lightDirection;

	lightDirection = vec_sub(light.pos, phit);
	lightDirection = vec_normalize(lightDirection);
	return (max(0.0f, dot_product(lightDirection, nhit)));
}

float calculatePhong(t_vec sphereCenter, t_vec intersection, t_vec lightPosition, t_vec rayOrigin)
{
	t_material sphereMaterial = { 5.0, 100.0 };

	t_vec sphereNormal = vec_sub(intersection, sphereCenter);
	sphereNormal = vec_normalize(sphereNormal);


	t_vec lightDirection = vec_sub(lightPosition, intersection);
	lightDirection = vec_normalize(lightDirection);


	t_vec viewDirection = vec_sub(intersection, rayOrigin);
	viewDirection = vec_normalize(viewDirection);


	t_vec blinnDirection = vec_sub(lightDirection, viewDirection);
	blinnDirection = vec_normalize(blinnDirection);

	float blinnTerm = max(dot_product(blinnDirection, sphereNormal), 0.0f);
	return sphereMaterial.specValue * powf(blinnTerm, sphereMaterial.specPower);
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
	    return (0);
	t_vec surface_color = {0, 0, 0};

	t_vec phit = vec_add(rayorig, vec_mult_f(raydir, tnear));
	t_vec nhit = vec_sub(phit, sphere->pos);
	nhit = vec_normalize(nhit);

    for (int i = 0; i < data->spheres.nb_spheres; i++)
    {
    	if (data->spheres.spheres[i].is_light == 1)
    	{
			int transmission = 1;
			t_vec lightDirection = vec_sub(data->spheres.spheres[i].pos, phit);
			lightDirection = vec_normalize(lightDirection);

			for (unsigned j = 0; j < data->spheres.nb_spheres; j++)
			{
				if (i != j)
				{
				    if (hitsphere(vec_add(phit, nhit), lightDirection, data->spheres.spheres[j], &t0, &t1) == 1)
				    {
				        transmission = 0;
				        break;
				    }
				}
			}

			if (transmission == 1)
			{
			float lambert = calculateLambert(phit, nhit, data->spheres.spheres[i]);
			float phongTerm = calculatePhong(sphere->pos, phit, data->spheres.spheres[i].pos, rayorig);

//float calculatePhong(t_vec sphereCenter, t_vec intersection, t_vec lightPosition, t_vec rayOrigin)

			surface_color = vec_add(vec_mult_f(sphere->surf_color, lambert),
			vec_mult_f(sphere->surf_color, phongTerm));
			}

			//surface_color = vec_mult(vec_mult_f(vec_mult_f(sphere->surf_color, transmission), max(0.0f, dot_product(nhit, lightDirection))), data->spheres.spheres[i].emis_color);
		}
	}

	//surface_color = vec_add(surface_color, sphere->emis_color);

	float red = min(1.0f, surface_color.x) * 255;
	float green = min(1.0f, surface_color.y) * 255;
	float blue = min(1.0f, surface_color.z) * 255;

	return ((int)(red) << 24 | (int)(green) << 16 | (int)(blue) << 8 | 255);
}

void
render(t_data *data)
{
	int rx = SDL_RX SUPERSAMPLING;
	int ry = SDL_RY SUPERSAMPLING;

	float invWidth = 1.0f / (float)rx;
	float invHeight = 1.0f / (float)ry;
	float fov = 30.0f;
	float aspectratio = rx / (float)ry;
	float angle = tan(M_PI * 0.5f * fov / 180.0f);

	for (int y = 0; y < ry; y++)
	{
		for (int x = 0; x < rx; x++)
		{
            float xx = (2.0f * ((x + 0.5f) * invWidth) - 1.0f) * angle * aspectratio;
            float yy = (1.0f - 2.0f * ((y + 0.5f) * invHeight)) * angle;
            t_vec raydir = {xx, yy, -1};

            raydir = vec_add(raydir, set_vec(0.0f, -0.2f, 0.0f));

            raydir = vec_normalize(raydir);
            t_vec rayorig = {0, 5, 10.0f};

            esdl_put_pixel(data->surf, x, y, raytrace(rayorig, raydir, data));
		}
	}
}

void				display(t_data *data)
{

	SDL_Texture		*texture = SDL_CreateTextureFromSurface(data->esdl->en.ren, data->surf);
	SDL_RenderClear(data->esdl->en.ren);
	SDL_Rect rect = {0, 0, SDL_RX, SDL_RY};
	SDL_RenderCopy(data->esdl->en.ren, texture, NULL, NULL);
	SDL_RenderPresent(data->esdl->en.ren);
	SDL_DestroyTexture(texture);
}

void				init(t_data *data)
{
	data->surf = esdl_create_surface(SDL_RX SUPERSAMPLING, SDL_RY SUPERSAMPLING);
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

	init_spheres(6, &data.spheres);

	int i = 0;
    data.spheres.spheres[i++] = set_sphere(set_vec( 0.0, -10004, -20), 10000, set_vec(0.20, 0.20, 0.20));
    data.spheres.spheres[i++] = set_sphere(set_vec( 0.0,      0, -20),     4, set_vec(1.00, 0.32, 0.36));
    data.spheres.spheres[i++] = set_sphere(set_vec( 5.0,     -1, -15),     2, set_vec(0.90, 0.76, 0.46));
    data.spheres.spheres[i++] = set_sphere(set_vec( 5.0,      0, -25),     3, set_vec(0.65, 0.77, 0.97));
    data.spheres.spheres[i++] = set_sphere(set_vec(-5.5,      0, -15),     3, set_vec(0.90, 0.90, 0.90));
    // light
    data.spheres.spheres[i++] = set_light(set_vec(0.0f,     20.0f, -30.0f),     3, set_vec(2.00, 2.00, 2.00));

	if (esdl_init(&esdl, 1024, 768, "Engine") == -1)
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
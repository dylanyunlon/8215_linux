#ifdef GL_ES

#ifdef GL_FRAGMENT_PRECISION_HIGH
precision	highp		float; 
precision	highp		int;
#else
precision	mediump		float; 
precision	mediump		int;
#endif

#endif
 

const	int c_zero_int				= 0;
const	int c_one_int				= 1;
const	float c_zero_float			= 0.0;
const	float c_one_float			= 1.0;
const	float c_min_float			= 0.000001;

/*[0--30)*/
const	int	draw_image_type			= 0;
const	int	draw_image_list_type	= 1;
const	int	draw_text_type			= 2;
const	int	draw_scroll_type		= 3;
const	int	draw_cube_image_type	= 4;
const	int	draw_spline_image_type	= 5;
const	int	draw_image_a8_type		= 6;/*2020 06 08*/

/*fill color [30--60)*/
const	int	draw_cube_color_type			= 30;
const	int	draw_fill_color_type			= 31;
const	int	draw_spline_color_type			= 32;
const	int	draw_cube_3d_color_type			= 33;
const	int	draw_cube_3d_texture_type		= 34;
const	int	draw_etc1_alpha_type			= 35;

const	int	draw_3d_model_color_type		= 36;
const	int	draw_3d_model_texture_type		= 37;
const	int	draw_3d_model_pickup_type		= 38;
const	int	draw_3d_model_one_color_type	= 39;
const	int	draw_3d_model_line_type			= 40;
const	int	draw_3d_model_point_type		= 41;
const	int	draw_draw_border_type			= 42;	
const	int	draw_3d_model_shadow_type		= 43;



/*custom [60--160)*/
const	int	draw_center_scale_custom_type	= 60;
const	int	hmi_mask_enable					= 512;
const	int	hmi_cast_on						= 64;

/*light*/
uniform		vec3		u_camera_pos; 			/*camera position*/	
uniform		int			u_light_type;			/*0:direct,1:spot light,2:point light,-1:no light,-2:pickup*/
uniform		int			u_light_shadow_type;	/*0:No shadow,1:soft shadow,2:hard shadow*/

uniform		vec4		u_one_color;

uniform		vec3		u_light_direction; 		/* 0,sun light:light direct. 1,spot:light  position*/	
uniform		vec4		u_light_ambient_color;
uniform		vec4		u_light_diffuse_color;
uniform		vec4		u_light_specular_color;

uniform		vec4 		u_material_ambient_color;
uniform		vec4 		u_material_diffuse_color;
uniform		vec4 		u_material_specular_color;

/*varying<=8,attribute number <=8,uniform element <=128*/
uniform		mat4		u_mvpMatrix; 
uniform		mat2		u_texMatrix;
uniform		int			u_draw_type;
uniform		float		u_light_specular_exponent;

uniform		mat4		u_mToWorldMatrix; /*mode*/


attribute	vec3		a_position; 
attribute	vec2		a_texCoord; 
attribute	vec2		a_etc1_alpha_texCoord; 
attribute	vec3		a_normal;
attribute	vec4		a_color;

varying	vec2			v_texCoord;   
varying	vec2			v_etc1_alpha_texCoord;  
varying	vec4			v_color;

/*blur*/
uniform		vec2		resolution;
uniform		float		blur_size;
uniform		int			u_qd;
uniform		int			u_attribute;
#pragma STDGL invariant(all)
invariant gl_Position;


#if 0
varying		int		v_attribute;
varying		int		v_draw_type;
#endif

vec4	qd_vertex_light_color(int itype/*0: color vertex,1:texture,2:one color*/)
{	
	vec4	computed_color	= vec4(c_zero_float, c_zero_float, c_zero_float, c_zero_float);	
	vec3	halfplane		= vec3(c_zero_float, c_zero_float, c_zero_float);
	vec3	peye			= vec3(c_zero_float, c_zero_float, c_zero_float);					
	float	ndotl			= c_zero_float;
	float	ndoth			= c_zero_float;
	vec3	vertex_world_position;
	vec3	normal;
	vec3	vp;
	
	vertex_world_position	= (u_mToWorldMatrix * vec4(a_position,c_one_float)).xyz;	

	normal					= a_position + a_normal;
	normal					= (u_mToWorldMatrix * vec4(normal,c_one_float)).xyz - vertex_world_position;		
	normal					= normalize(normal);

	if(u_light_type == 0)// direct light
	{
		peye			= normalize(u_camera_pos - vertex_world_position);
		vp				= -u_light_direction;
		halfplane		= vp + peye;
	}
	else if(u_light_type == 1)//spot light
	{
		peye			= normalize(u_camera_pos - vertex_world_position);
		vp				= normalize(u_light_direction - vertex_world_position);
		halfplane		= vp + peye;			
	}
	else if(u_light_type == 2)//point light
	{
		peye			= normalize(u_camera_pos - vertex_world_position);
		vp				= normalize(u_light_direction - vertex_world_position);
		halfplane		= vp + peye;
	}
	else
	{
	}
	
	if(itype == 0)	/*color vertex*/
	{
		computed_color += (a_color * u_light_ambient_color);

		computed_color += (ndotl * a_color *	
							u_light_diffuse_color );
								
		computed_color += (pow(ndoth, 2.0/*u_material.specular_exponent*/) *
							u_light_specular_color);
	}
	else if((itype == 1)	/*texture vertex*/)
	{
		ndotl			= max(c_zero_float, dot(normal, vp));	
		ndoth			= max(c_zero_float, dot(normal,halfplane));
		
		computed_color += (u_light_ambient_color * u_material_ambient_color);
		computed_color += (ndotl * u_material_diffuse_color *	
						u_light_diffuse_color );
							
		computed_color += (pow(ndoth, 2.0/*u_material.specular_exponent*/) *
						u_material_specular_color *
						u_light_specular_color);
	}
	else	/*one color*/
	{				
		ndotl			= max(c_zero_float, dot(normal, vp));	
		ndoth			= max(c_zero_float, dot(normal,halfplane));

		//computed_color += u_one_color;
		computed_color += (u_light_ambient_color * u_material_ambient_color);

		computed_color += (ndotl * u_light_diffuse_color *	
							 u_material_diffuse_color);
		
		computed_color += (pow(ndoth, 2.0/*u_material.specular_exponent*/) *
							u_material_specular_color * u_light_specular_color);
		computed_color	= computed_color * u_one_color;
	}
	
		
	return computed_color;
}


void main()                  
{    	
	bool	bshadow_on		= false;
	int		iattribute;

	iattribute	= u_attribute;
	/*MASK*/	
	if(iattribute >= hmi_mask_enable)
	{		
		iattribute		-= hmi_mask_enable;		
	}
	if(iattribute >= hmi_cast_on)
	{	
		iattribute		-= hmi_cast_on;	
		bshadow_on		= true;
	}
	if(u_draw_type < draw_cube_color_type/*draw_center_scale_custom_type*/)
	{
		gl_Position	= u_mvpMatrix * vec4(a_position,c_one_float);			
		v_texCoord	= a_texCoord;									
	}
	else if(u_draw_type == draw_etc1_alpha_type)
	{
		gl_Position	= u_mvpMatrix * vec4(a_position,c_one_float);	
		v_texCoord	= a_texCoord;	
		v_etc1_alpha_texCoord	= a_etc1_alpha_texCoord;	/*etc1 alpha UV*/		
	}
	else if(u_draw_type == draw_3d_model_color_type)
	{
		gl_Position	= u_mvpMatrix * vec4(a_position,c_one_float);		
		if((u_light_type >= 0)&&(bshadow_on))/*light enable*/
		{
			v_color		= qd_vertex_light_color(0/*color vertex*/);
		}
		else
		{
			v_color		= a_color;
		}
	}
	else if(u_draw_type == draw_3d_model_shadow_type)
	{
		gl_Position	= u_mvpMatrix * vec4(a_position,c_one_float);
		v_color		=  u_light_ambient_color;
	}
	else if(u_draw_type == draw_3d_model_texture_type)
	{
		gl_Position	= u_mvpMatrix * vec4(a_position,c_one_float);
		v_texCoord	= a_texCoord;
		if((u_light_type >= 0)&&(bshadow_on))/*light enable*/
		{
			v_color		= qd_vertex_light_color(1/*texture */);
		}		
	}
	else if(u_draw_type == draw_3d_model_one_color_type)
	{
		gl_Position	= u_mvpMatrix * vec4(a_position,c_one_float);
		if((u_light_type >= 0)&&(bshadow_on))/*light enable*/
		{
			v_color		= qd_vertex_light_color(2/*one color */);
		}	
		else
		{
			v_color		= u_one_color;
		}
	}
	else if(u_draw_type == draw_3d_model_pickup_type)
	{
		gl_Position	= u_mvpMatrix * vec4(a_position,c_one_float);								
	}
	else if(u_draw_type == draw_spline_color_type)/*spline color*/
	{
		gl_Position	= u_mvpMatrix * vec4(a_position,c_one_float);
	}
	else if(u_draw_type == draw_cube_3d_color_type)/*3d color cube model*/
	{
	#if 0
		gl_Position	= u_mvpMatrix * vec4(a_position,c_one_float);	
		if(u_light_type >= 0)/*light enable*/
		{
			light_color	= qd_vertex_light_color(0/*color vertex*/);
			v_color		= a_color * light_color;			
		}
	#endif
	}
	else if(u_draw_type == draw_cube_3d_texture_type)/*3d cube model*/
	{
	#if 0
		gl_Position	= u_mvpMatrix * vec4(a_position,c_one_float);	
		v_texCoord	= a_texCoord;
		if(u_light_type >= 0)/*light enable*/
		{
			v_color		= qd_vertex_light_color(1/*texture */);
		}	
	#endif
	}	
	else if(u_draw_type == draw_3d_model_line_type)
	{
		gl_Position	= u_mvpMatrix * vec4(a_position,c_one_float);						
	}
	else if(u_draw_type == draw_3d_model_point_type)
	{
		gl_Position	= u_mvpMatrix * vec4(a_position,c_one_float);	
	}
	else if(u_draw_type == draw_draw_border_type)
	{
		gl_Position	= u_mvpMatrix * vec4(a_position,c_one_float);	
	}
	else if(u_draw_type == draw_fill_color_type)
	{
		gl_Position	= u_mvpMatrix * vec4(a_position,c_one_float);				
		v_color		= a_color;
	}
	else if(u_draw_type < draw_center_scale_custom_type)
	{
		gl_Position	= u_mvpMatrix * vec4(a_position,c_one_float);				
		v_color		= u_one_color;
	}
	else
	{
		gl_Position	= u_mvpMatrix * vec4(a_position,c_one_float);			
	}	
}  



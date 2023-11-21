#include "core.h"
u32 box_indices[] = {
	8, 1, 10, 
	11, 5, 15, 
	14, 7, 2, 
	12, 4, 9, 
	12, 0, 2, 
	1, 4, 6, 
	6, 3, 1, 
	2, 16, 12, 
	9, 8, 12, 
	2, 3, 14, 
	15, 13, 11, 
	10, 2, 8, 
};

f32 box_vertices[] = {
	0.5f, -0.5f, 0.5f, 0.9960938096046448f, 0.9960936903953552f, 0.0f, 0.5773503184318542f, -0.5773503184318542f, 0.577350378036499f,
	-0.5f, -0.5f, 0.5f, 0.0f, 0.9960936903953552f, 0.0f, -0.5773503184318542f, -0.5773503184318542f, 0.5773502588272095f,
	0.5f, 0.5f, 0.5f, 0.9960938096046448f, 0.0f, 0.0f, 0.5773503184318542f, 0.5773503184318542f, 0.5773502588272095f,
	-0.5f, 0.5f, 0.5f, 6.099314846588766e-17f, 0.0f, 0.0f, -0.5773503184318542f, 0.5773503184318542f, 0.577350378036499f,
	-0.5f, -0.5f, -0.5f, 0.9960938096046448f, 0.9960936903953552f, 0.0f, -0.5773503184318542f, -0.5773503184318542f, -0.577350378036499f,
	0.5f, -0.5f, -0.5f, 0.9960938096046448f, 0.9960936903953552f, 0.0f, 0.5773503184318542f, -0.5773503184318542f, -0.5773502588272095f,
	-0.5f, 0.5f, -0.5f, 0.9960938096046448f, 0.0f, 0.0f, -0.5773503184318542f, 0.5773503184318542f, -0.5773502588272095f,
	0.5f, 0.5f, -0.5f, 0.9960938096046448f, 0.9960936903953552f, 0.0f, 0.5773503184318542f, 0.5773503184318542f, -0.577350378036499f,
	0.5f, -0.5f, 0.5f, 6.099314846588766e-17f, 0.0f, 0.0f, 0.5773503184318542f, -0.5773503184318542f, 0.577350378036499f,
	-0.5f, -0.5f, 0.5f, 0.9960938096046448f, 0.0f, 0.0f, -0.5773503184318542f, -0.5773503184318542f, 0.5773502588272095f,
	-0.5f, 0.5f, 0.5f, 0.9960938096046448f, 0.9960936903953552f, 0.0f, -0.5773503184318542f, 0.5773503184318542f, 0.577350378036499f,
	-0.5f, -0.5f, -0.5f, 0.0f, 0.9960936903953552f, 0.0f, -0.5773503184318542f, -0.5773503184318542f, -0.577350378036499f,
	0.5f, -0.5f, -0.5f, 0.0f, 0.9960936903953552f, 0.0f, 0.5773503184318542f, -0.5773503184318542f, -0.5773502588272095f,
	-0.5f, 0.5f, -0.5f, 6.099314846588766e-17f, 0.0f, 0.0f, -0.5773503184318542f, 0.5773503184318542f, -0.5773502588272095f,
	-0.5f, 0.5f, -0.5f, 0.0f, 0.9960936903953552f, 0.0f, -0.5773503184318542f, 0.5773503184318542f, -0.5773502588272095f,
	0.5f, 0.5f, -0.5f, 0.9960938096046448f, 0.0f, 0.0f, 0.5773503184318542f, 0.5773503184318542f, -0.577350378036499f,
	0.5f, 0.5f, -0.5f, 6.099314846588766e-17f, 0.0f, 0.0f, 0.5773503184318542f, 0.5773503184318542f, -0.577350378036499f,
};

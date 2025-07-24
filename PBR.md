# Rendering equation

The rendering equation is pretty much just this:

outgoing light = emitted light + reflected light

The equation defines what color any point on an object would be.
The reflected light is the light that the object reflects.
The emitted light is the light that the object emits, this would be useful for lights
and emissive surfaces but we are not working with emissive surfaces here so we can 
remove that part of the equation, so now it's this.

outgoing light = reflected light

Let's put it into more mathematical terms:

Lo = Li

But running this function a white sphere with a single light source emitting white
light will get you a completely white sphere which more closely resembles a circle more
than anything. Spheres in real life have their surfaces get darker if they are pointed away
from the light because light rays can't reach them there. So let's adjust this equation a
little so it would make a point on a surface get darker if it points away from the light
source:

Lo = Li * dot(N, Li)

The N vector is the normal, it defines the direction in which the point is pointing in.
The dot product gives a number which gets higher the more two vectors point away from
each other. But running this function on a sphere will give you backwards lighting, where
the sphere gets brighter the more the surface points away, and this makes sense since we
are multiplying with the dot product which gives a higher result the more two vectors point
away. We have to invert the incoming light vector to get the correct result.

Lo = Li * dot(N, inverse(Li))

And we have correct lighting. This function works brilliantly if the object you want to 
render is a perfectly diffuse object with no specularity, imperfections or any materials.
But things in real life are often not that and their materials are often wildly different to
each other. We could make a different function for each material we find in our lives but
let's just make an abstracted function for now, let's call it the BRDF function.

Lo = BRDF(x, Di, Do) * Li * dot(N, inverse(Li))

x is the position of the point, Di is the incoming direction, Do is the outgoing direction.
Let's also add the emitted light back in:

Lo = Le + BRDF(x, Di, Do) * Li * dot(N, inverse(Li))

We often accumulate this equation for multiple lights in a scene so people usually add a 
summation operator at the beginning of the equation.

# PBR

The basis of physically based rendering is microfacet theory.

An object in real life has thousands, even millions of tiny little bumps, the wilder the
bumps, the rougher the object is. These are called microfacets according to PBR,
and specular highlights would be dependant on how many microfacets point close enough
to the halfway vector between the view position and the surface normal.

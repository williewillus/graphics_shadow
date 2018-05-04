Names:
- Vincent Lee (vl3685)
- John Fang (jf35533)

Building:
- Setup cmake and compile like standard project
- Run `bin/shadow <OBJ file>`
- The OBJ mesh must be closed

Controls:
- Z: switches between techniques (shadow volume, ssao, shadow map)
- C/V: selects the previous/next light
- X: toggles debug mode
  - in shadow volume mode, visualizes the volumes for the current light
  - in ssao mode, visualizes the raw ssao attenuation texture
  - in shadow map mode, visualizes the shadow map for the current light
- 1/2/3/4/5/6: moves current light in the -X/+X/-Y/+Y/-Z/+Z direction

Extra Credit:
- screen space AO

Report:
- See report.pdf

We affirm that we have done the course instructor survey online for 10 extra points.

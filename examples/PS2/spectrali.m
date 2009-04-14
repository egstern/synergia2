turns=input("Enter the number of turns: ")
mix=load("mi_x.dat");
miy=load("mi_y.dat");
#miz=load("mi_z.dat");
sz=size(mix)(1);
dk=1./turns;
kicks=sz/turns
axis([-kicks/2-1,kicks/2+1])
#for i=1:sz
#n(i)=(i-1)*dk;
#endfor
clear n;
n=[0:sz-1]*dk;
ffx=fft(mix(:,4));
ffy=fft(miy(:,4));
#ffz=fft(miz(:,4));
hold off
clear n1;
n1=[-sz/2:sz/2-1]*dk;
for i=1:sz/2
ff1x(i)=ffx(i+sz/2);
ff1x(i+sz/2)=ffx(i);
ff1y(i)=ffy(i+sz/2);
ff1y(i+sz/2)=ffy(i);
#ff1z(i)=ffz(i+sz/2);
#ff1z(i+sz/2)=ffz(i);
endfor
plot(n1,abs(ff1x))
hold on
plot(n1,abs(ff1y))
#plot(n1,abs(ff1z))

#clear m;
#m=ones(1,kicks/8);
#m=m*1e-12;
#clear nm;
#for i=1:kicks/8
#nm(i)=8*(i-kicks/16)+2;#*2*pi/(length);
#endfor
#plot(nm,m,'^')


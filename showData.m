function showData
global t ekin epot etherm klein
t = []; ekin = []; epot = []; etherm = []; klein = 1e-30;
files = dir('data/partData.*');
k=1;
while k<=numel(files)
    if mod(k,1)==0
        filenameP = [ 'data/', files(k).name ];
        showParticles( filenameP );
        filenameF = strrep(filenameP, 'part', 'field');
        showFields( filenameF );
        pause(.2)
    end
    files = dir('data/partData.*');
    k = k+1;
end
% for k = 1:numel(files)
%     if mod(k,10)==0
%         filenameP = [ 'data/', files(k).name ];
%         showParticles( filenameP );
%         filenameF = strrep(filenameP, 'part', 'field');
%         showFields( filenameF );
%         pause(.2)
%     end
% end
end

function showParticles( fileName )
global klein
fid = fopen(fileName, 'r');
np = fscanf(fid, '# np = %d', [1 1]);
time = fscanf(fid, ' time = %g *', [1 1]);
pData = fread(fid, [4, np], 'double');
fclose(fid);
xp = pData(1, :);
vp = pData(2, :);
mp = pData(3, :);
qp = pData(4, :);

yl = max(max(vp),max(-vp))+klein;
c = qp;
subplot(4, 2, 1);
scatter( xp, vp, [], c, '.' );
title( [ 'time = ', num2str(time) ] );
xlabel('x');
ylabel('v');
xlim([0, 3]);
ylim([-yl, yl]);
end

function showFields( fileName )
global t ekin epot etherm klein

fid = fopen(fileName, 'r');

nx = fscanf(fid, '# nx = %d', [1 1]);
time = fscanf(fid, ' time = %g *', [1 1]);
f = fread(fid, [nx, 8], 'double'); % fields x, n, rho, u, T, Ex
fclose(fid);
x = f(:, 1);
n = f(:, 2);
mu = f(:, 3);
rho = f(:, 4);
u = f(:, 5);
nu = f(:, 6);
T = f(:, 7);
Ex = f(:, 8);

dx = x(2) - x(1);   

subplot(4, 2, 2);
plot( x, n, '-' );
title('n');

subplot(4, 2, 3);
plot( x, u, '-' );
title('u');

subplot(4, 2, 4);
plot( x, T, '-' );
title('T');

subplot(4, 2, 5);
plot( x - 0.*dx, Ex, '-' );
title('Ex');
yl = max(max(Ex),max(-Ex))+klein;
ylim([-yl, yl]);

subplot(4, 2, 6);
plot( x, rho, '-' );
title('rho');
yl = max(max(rho),max(-rho))+klein;
ylim([-yl, yl]);


eth = dx * sum( n .* T );
ek = dx * sum( nu .* nu./(2.* mu) );
ep = dx * sum( Ex.^2 );
t = [ t ; time ];
ekin = [ ekin ; ek ];
epot = [ epot ; ep ];
etherm = [ etherm ; eth ];
subplot(4, 2, [7 8]);
%  plot(t, ekin+epot+etherm);
plot(t, ekin, t, epot, t, ekin+epot+etherm, t, etherm);
legend('E_{kin}', 'E_{pot}', 'E_{tot}', 'E_{therm}', 'Location','best');

end

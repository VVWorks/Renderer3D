
inline int clamp(int min, int max, int n) {
	if (n <= min) { return min - 1; }
	if (n >= max) { return max - 1; }
	return n;
}

int middle(int x1, int x2, int x3) {
	int a = max(max(x1, x2), x3);
	int b = min(min(x1, x2), x3);
	if(x1!=a && x1!=b){
		return x1;
	}
	if (x2 != a && x2 != b) {
		return x2;
	}
	if (x3 != a && x3 != b) {
		return x3;
	}
}